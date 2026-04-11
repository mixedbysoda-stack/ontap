#pragma once
#include <juce_dsp/juce_dsp.h>

class BandSplit
{
public:
    BandSplit();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setFrequency(float freqHz);
    void setActive(bool active);
    bool getIsActive() const { return isActive; }

    // Split the buffer into low/high. Call applyDucking on lowBand yourself, then call recombine.
    void split(juce::AudioBuffer<float>& buffer);
    void recombine(juce::AudioBuffer<float>& buffer);

    juce::AudioBuffer<float>& getLowBand() { return lowBand; }

private:
    bool isActive = false;
    float cutoffFreq = 320.0f;
    double sampleRate = 44100.0;
    int preparedBlockSize = 0;
    int preparedChannels = 0;

    juce::dsp::LinkwitzRileyFilter<float> lowpass;
    juce::dsp::LinkwitzRileyFilter<float> highpass;

    juce::AudioBuffer<float> lowBand;
    juce::AudioBuffer<float> highBand;

    int currentNumSamples = 0;
};
