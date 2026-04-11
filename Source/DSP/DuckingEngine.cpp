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

void DuckingEngine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    bandSplit.prepare(spec);
    currentPhase = 0.0f;
    smoothedGain = 1.0f;
    triggerPhase = 0.0f;
    audioEnvelopeFollower = 0.0f;
    audioTriggerArmed = true;
}

void DuckingEngine::reset()
{
    bandSplit.reset();
    currentPhase = 0.0f;
    smoothedGain = 1.0f;
    triggerPhase = 0.0f;
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

void DuckingEngine::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi,
                                 double bpm, double ppqPosition, bool isPlaying)
{
    if (bypassParam->load() > 0.5f)
        return;

    updateParameters();

    const int numSamples = buffer.getNumSamples();

    // Handle trigger modes
    if (currentTriggerMode == 0) // Sync
    {
        currentPhase = getPhaseFromSync(ppqPosition, isPlaying);
    }
    else if (currentTriggerMode == 1) // MIDI
    {
        for (const auto metadata : midi)
        {
            auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                triggered = true;
                triggerPhase = 0.0f;
            }
        }
    }
    // Audio trigger mode (mode 2) would use sidechain input
    // For now, phase advances based on trigger state

    // Apply ducking — with or without band split
    if (bandSplit.getIsActive())
    {
        bandSplit.split(buffer);
        applyDucking(bandSplit.getLowBand(), buffer.getNumSamples());
        bandSplit.recombine(buffer);
    }
    else
    {
        applyDucking(buffer, buffer.getNumSamples());
    }
}

void DuckingEngine::applyDucking(juce::AudioBuffer<float>& buffer, int samplesToProcess)
{
    const int numSamples = samplesToProcess;
    const int numChannels = buffer.getNumChannels();
    float lengthBeats = getLengthInBeats();

    // Calculate phase increment per sample for MIDI/Audio modes
    // (Sync mode uses ppqPosition directly, but we still need per-sample values)
    float phaseIncrement = 0.0f;
    if (currentTriggerMode != 0)
    {
        // Use a reasonable default BPM for phase calculation
        float beatsPerSecond = 120.0f / 60.0f; // fallback
        phaseIncrement = beatsPerSecond / (static_cast<float>(sampleRate) * lengthBeats);
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float phase;

        if (currentTriggerMode == 0) // Sync
        {
            // Phase is set per-block from ppqPosition, interpolate within block
            phase = currentPhase + (static_cast<float>(sample) / numSamples) *
                    (1.0f / static_cast<float>(numSamples)); // tiny advance
            phase = std::fmod(phase + 100.0f, 1.0f);
        }
        else // MIDI or Audio
        {
            if (triggered)
            {
                phase = triggerPhase;
                triggerPhase += phaseIncrement;

                if (triggerPhase >= 1.0f)
                {
                    if (looping)
                        triggerPhase -= 1.0f;
                    else
                    {
                        triggered = false;
                        triggerPhase = 1.0f;
                    }
                }
            }
            else
            {
                phase = 1.0f; // No ducking when not triggered
            }
        }

        // Look up the ducking curve value
        float curveValue = curvePresets.getValue(currentPreset, phase, currentSlope, currentShift);

        // curveValue: 1.0 = full volume at start of curve, 0.0 = fully ducked
        // We want to invert: at the start of the curve, the signal should be ducked DOWN
        float duckGain = 1.0f - curveValue;

        // Apply mix
        float targetGain = 1.0f - (currentMix * (1.0f - duckGain));

        // Anti-click smoothing
        float smoothCoeff = 1.0f / antiClickSmoothing;
        smoothedGain += (targetGain - smoothedGain) * smoothCoeff;

        // Apply gain to all channels
        for (int ch = 0; ch < numChannels; ++ch)
        {
            buffer.getWritePointer(ch)[sample] *= smoothedGain;
        }
    }
}
