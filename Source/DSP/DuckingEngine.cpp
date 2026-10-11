#include "DuckingEngine.h"
#include "../Parameters/ParameterIDs.h"
#include <cmath>

DuckingEngine::DuckingEngine(juce::AudioProcessorValueTreeState& apvts)
{
    mixParam         = apvts.getRawParameterValue(ParameterIDs::Mix::mix.getParamID());
    presetParam      = apvts.getRawParameterValue(ParameterIDs::Curve::presetIndex.getParamID());
    slopeParam       = apvts.getRawParameterValue(ParameterIDs::Curve::slopeOffset.getParamID());
    shiftParam       = apvts.getRawParameterValue(ParameterIDs::Curve::shiftAmount.getParamID());
    lengthParam      = apvts.getRawParameterValue(ParameterIDs::Timing::length.getParamID());
    triggerModeParam = apvts.getRawParameterValue(ParameterIDs::Timing::triggerMode.getParamID());
    loopParam        = apvts.getRawParameterValue(ParameterIDs::Timing::loop.getParamID());
    bandActiveParam  = apvts.getRawParameterValue(ParameterIDs::Band::active.getParamID());
    bandFreqParam    = apvts.getRawParameterValue(ParameterIDs::Band::frequency.getParamID());
    bypassParam      = apvts.getRawParameterValue(ParameterIDs::Global::bypass.getParamID());
    antiClickParam   = apvts.getRawParameterValue(ParameterIDs::Global::antiClick.getParamID());
}

namespace
{
    constexpr float sidechainSilenceLevel = 3.1623e-5f; // -90 dBFS

    // End of the curve. CurvePresets wraps the position with fmod(position + 10, 1) in float,
    // which turns any phase within ~5e-7 of 1.0 into 0.0 (the fully ducked start), so a curve
    // ending exactly on a sample would otherwise click down for one sample.
    constexpr float maxPhase = 0.999999f;

    float onePoleCoeff(double seconds, double sampleRate)
    {
        return static_cast<float>(1.0 - std::exp(-1.0 / (seconds * sampleRate)));
    }

    float followEnvelope(float envelope, float input, float attackCoeff, float releaseCoeff)
    {
        return envelope + (input - envelope) * (input > envelope ? attackCoeff : releaseCoeff);
    }
}

void DuckingEngine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    bandSplit.prepare(spec);

    envAttackCoeff   = onePoleCoeff(0.001, sampleRate); // 1 ms
    envReleaseCoeff  = onePoleCoeff(0.080, sampleRate); // 80 ms
    slowAttackCoeff  = onePoleCoeff(0.050, sampleRate); // 50 ms
    slowReleaseCoeff = onePoleCoeff(0.300, sampleRate); // 300 ms
    sidechainSilenceHoldSamples = static_cast<int>(0.3 * sampleRate);

    reset();
}

void DuckingEngine::reset()
{
    bandSplit.reset();
    currentPhase = 0.0f;
    smoothedGain = 1.0f;
    triggered = false;
    triggerPhase = 0.0;
    audioEnvelopeFollower = 0.0f;
    audioTriggerArmed = true;
    mainFastEnvelope = 0.0f;
    mainSlowEnvelope = 0.0f;
    mainTriggerArmed = true;
    sidechainSilentSamples = 0;
}

void DuckingEngine::updateParameters()
{
    currentPreset = static_cast<int>(presetParam->load());
    currentMix = mixParam->load() / 100.0f;
    currentSlope = slopeParam->load();
    currentShift = shiftParam->load();
    currentTriggerMode = static_cast<int>(triggerModeParam->load());
    looping = loopParam->load() > 0.5f;

    bandSplit.setActive(bandActiveParam->load() > 0.5f);
    bandSplit.setFrequency(bandFreqParam->load());

    // Anti-click smoothing coefficient
    bool antiClick = antiClickParam->load() > 0.5f;
    antiClickSmoothing = antiClick ? (6.0f / 1000.0f * static_cast<float>(sampleRate)) : 1.0f;
    if (antiClickSmoothing < 1.0f) antiClickSmoothing = 1.0f;
}

float DuckingEngine::getLengthInBeats() const
{
    int lengthIndex = static_cast<int>(lengthParam->load());
    switch (lengthIndex)
    {
        case 0: return 0.5f;   // 1/8 note
        case 1: return 1.0f;   // 1/4 note (default)
        case 2: return 2.0f;   // 1/2 note
        case 3: return 4.0f;   // 1/1 (whole note)
        default: return 1.0f;
    }
}

float DuckingEngine::getPhaseFromSync(double ppqPosition, bool isPlaying) const
{
    if (!isPlaying) return 0.0f;

    float lengthBeats = getLengthInBeats();
    double posInCycle = std::fmod(ppqPosition, static_cast<double>(lengthBeats));
    return static_cast<float>(posInCycle / lengthBeats);
}

void DuckingEngine::processBlock(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& sidechain,
                                 juce::MidiBuffer& midi, double bpm, double ppqPosition, bool isPlaying)
{
    if (bypassParam->load() > 0.5f)
        return;

    updateParameters();

    blockBpm = bpm > 0.0 ? bpm : 120.0;
    blockPpqPosition = ppqPosition;
    blockIsPlaying = isPlaying;

    // Apply ducking — with or without band split
    // (triggers are always detected on the full-band, pre-ducking input)
    if (bandSplit.getIsActive())
    {
        bandSplit.split(buffer);
        applyDucking(bandSplit.getLowBand(), buffer, sidechain, midi, buffer.getNumSamples());
        bandSplit.recombine(buffer);
    }
    else
    {
        applyDucking(buffer, buffer, sidechain, midi, buffer.getNumSamples());
    }
}

bool DuckingEngine::detectAudioTrigger(const juce::AudioBuffer<float>& input,
                                       const juce::AudioBuffer<float>& sidechain, int sample)
{
    // Sidechain key: mono sum, and per-channel peak for the silence check
    const int numSidechainChannels = sidechain.getNumChannels();
    float sidechainSum = 0.0f;
    float sidechainPeak = 0.0f;
    for (int ch = 0; ch < numSidechainChannels; ++ch)
    {
        const float x = sidechain.getReadPointer(ch)[sample];
        sidechainSum += x;
        sidechainPeak = std::max(sidechainPeak, std::abs(x));
    }
    const float sidechainKey = numSidechainChannels > 0 ? std::abs(sidechainSum) / static_cast<float>(numSidechainChannels) : 0.0f;

    // Main input key: mono sum
    const int numInputChannels = input.getNumChannels();
    float mainSum = 0.0f;
    for (int ch = 0; ch < numInputChannels; ++ch)
        mainSum += input.getReadPointer(ch)[sample];
    const float mainKey = numInputChannels > 0 ? std::abs(mainSum) / static_cast<float>(numInputChannels) : 0.0f;

    audioEnvelopeFollower = followEnvelope(audioEnvelopeFollower, sidechainKey, envAttackCoeff, envReleaseCoeff);
    mainFastEnvelope = followEnvelope(mainFastEnvelope, mainKey, envAttackCoeff, envReleaseCoeff);
    mainSlowEnvelope = followEnvelope(mainSlowEnvelope, mainKey, slowAttackCoeff, slowReleaseCoeff);

    // Fall back to the main input when the sidechain is disabled or silent for ~300 ms
    if (sidechainPeak > sidechainSilenceLevel)
        sidechainSilentSamples = 0;
    else if (sidechainSilentSamples < sidechainSilenceHoldSamples)
        ++sidechainSilentSamples;
    const bool useMainInput = sidechainSilentSamples >= sidechainSilenceHoldSamples;

    bool fire = false;

    // Sidechain: rising edge over the threshold, re-armed 6 dB below it
    if (audioTriggerArmed)
    {
        if (audioEnvelopeFollower > audioTriggerThreshold)
        {
            audioTriggerArmed = false;
            fire = ! useMainInput;
        }
    }
    else if (audioEnvelopeFollower < audioTriggerThreshold * 0.5f)
    {
        audioTriggerArmed = true;
    }

    // Main input: the full mix rarely drops below a fixed threshold, so look for a
    // transient instead (fast envelope 6 dB over the slow one), re-armed once it settles
    if (mainTriggerArmed)
    {
        if (mainFastEnvelope > audioTriggerThreshold && mainFastEnvelope > 2.0f * mainSlowEnvelope)
        {
            mainTriggerArmed = false;
            fire = fire || useMainInput;
        }
    }
    else if (mainFastEnvelope < 1.25f * mainSlowEnvelope)
    {
        mainTriggerArmed = true;
    }

    return fire;
}

void DuckingEngine::applyDucking(juce::AudioBuffer<float>& target, const juce::AudioBuffer<float>& input,
                                 const juce::AudioBuffer<float>& sidechain, const juce::MidiBuffer& midi,
                                 int samplesToProcess)
{
    const int numSamples = samplesToProcess;
    const int numChannels = target.getNumChannels();
    const double lengthBeats = static_cast<double>(getLengthInBeats());

    // Host tempo drives every mode: Sync reads the position, MIDI/Audio advance the curve
    // by one Length (in beats) at the host tempo
    const double beatsPerSample = blockBpm / (60.0 * sampleRate);
    const double phaseIncrement = beatsPerSample / lengthBeats;

    auto midiIt = midi.cbegin();
    const auto midiEnd = midi.cend();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Restart the curve at the exact sample of the trigger. With Loop on the curve
        // keeps cycling until the next trigger; with Loop off it plays once.
        bool restart = false;

        if (currentTriggerMode == 1) // MIDI
        {
            // Raw bytes rather than getMessage(), which can allocate for long SysEx
            for (; midiIt != midiEnd && (*midiIt).samplePosition <= sample; ++midiIt)
            {
                const auto event = *midiIt;
                if (event.numBytes >= 3 && (event.data[0] & 0xf0) == 0x90 && event.data[2] > 0)
                    restart = true; // note-on with velocity > 0, any channel
            }
        }
        else if (currentTriggerMode == 2) // Audio
        {
            // Input is read before this sample's gain is written, so the key is pre-ducking
            restart = detectAudioTrigger(input, sidechain, sample);
        }

        if (restart)
        {
            triggered = true;
            triggerPhase = 0.0;
        }

        float phase = 0.0f;
        bool active = true;

        if (currentTriggerMode == 0) // Sync
        {
            currentPhase = getPhaseFromSync(blockPpqPosition + sample * beatsPerSample, blockIsPlaying);
            phase = currentPhase;
        }
        else if (triggered) // MIDI or Audio
        {
            phase = static_cast<float>(triggerPhase);
            triggerPhase += phaseIncrement;

            if (triggerPhase >= 1.0)
            {
                if (looping)
                    triggerPhase -= 1.0;
                else
                {
                    triggered = false;
                    triggerPhase = 0.0;
                }
            }
        }
        else
        {
            active = false; // No ducking when not triggered
        }

        float targetGain = 1.0f;

        if (active)
        {
            // Look up the ducking curve value
            float curveValue = curvePresets.getValue(currentPreset, std::min(phase, maxPhase), currentSlope, currentShift);

            // curveValue: 1.0 = full volume at start of curve, 0.0 = fully ducked
            // We want to invert: at the start of the curve, the signal should be ducked DOWN
            float duckGain = 1.0f - curveValue;

            // Apply mix
            targetGain = 1.0f - (currentMix * (1.0f - duckGain));
        }

        // Anti-click smoothing
        float smoothCoeff = 1.0f / antiClickSmoothing;
        smoothedGain += (targetGain - smoothedGain) * smoothCoeff;

        // Apply gain to all channels
        for (int ch = 0; ch < numChannels; ++ch)
        {
            target.getWritePointer(ch)[sample] *= smoothedGain;
        }
    }
}
