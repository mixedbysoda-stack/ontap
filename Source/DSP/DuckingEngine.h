#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "CurvePresets.h"
#include "BandSplit.h"

class DuckingEngine
{
public:
    DuckingEngine(juce::AudioProcessorValueTreeState& apvts);

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    // buffer: the main bus (in place). sidechain: the sidechain input bus, which may
    // have zero channels when the host has it disabled.
    void processBlock(juce::AudioBuffer<float>& buffer, const juce::AudioBuffer<float>& sidechain,
                      juce::MidiBuffer& midi, double bpm, double ppqPosition, bool isPlaying);

    const CurvePresets& getCurvePresetsRef() const { return curvePresets; }

private:
    void applyDucking(juce::AudioBuffer<float>& target, const juce::AudioBuffer<float>& input,
                      const juce::AudioBuffer<float>& sidechain, const juce::MidiBuffer& midi,
                      int samplesToProcess);
    bool detectAudioTrigger(const juce::AudioBuffer<float>& input,
                            const juce::AudioBuffer<float>& sidechain, int sample);
    void updateParameters();
    float getPhaseFromSync(double ppqPosition, bool isPlaying) const;
    float getLengthInBeats() const;

    // Parameter pointers
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* presetParam = nullptr;
    std::atomic<float>* slopeParam = nullptr;
    std::atomic<float>* shiftParam = nullptr;
    std::atomic<float>* lengthParam = nullptr;
    std::atomic<float>* triggerModeParam = nullptr;
    std::atomic<float>* loopParam = nullptr;
    std::atomic<float>* bandActiveParam = nullptr;
    std::atomic<float>* bandFreqParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;
    std::atomic<float>* antiClickParam = nullptr;

    // DSP state
    CurvePresets curvePresets;
    BandSplit bandSplit;

    double sampleRate = 44100.0;
    float currentPhase = 0.0f;
    float smoothedGain = 1.0f;
    float antiClickSmoothing = 0.002f; // ~6ms at 44.1kHz

    // Host transport, latched per block
    double blockBpm = 120.0;
    double blockPpqPosition = 0.0;
    bool blockIsPlaying = false;

    // MIDI/Audio trigger state
    bool triggered = false;
    bool looping = true;
    double triggerPhase = 0.0;

    // Audio trigger: sidechain key (absolute threshold with hysteresis)
    float audioTriggerThreshold = 0.1f;
    float audioEnvelopeFollower = 0.0f;
    bool audioTriggerArmed = true;

    // Audio trigger fallback: main input key (transient = fast envelope 6 dB over slow)
    float mainFastEnvelope = 0.0f;
    float mainSlowEnvelope = 0.0f;
    bool mainTriggerArmed = true;
    int sidechainSilentSamples = 0;
    int sidechainSilenceHoldSamples = 14400;

    float envAttackCoeff = 0.0f;
    float envReleaseCoeff = 0.0f;
    float slowAttackCoeff = 0.0f;
    float slowReleaseCoeff = 0.0f;

    // Cached parameter values
    int currentPreset = 4;
    float currentMix = 1.0f;
    float currentSlope = 0.0f;
    float currentShift = 0.0f;
    int currentTriggerMode = 0;
};
