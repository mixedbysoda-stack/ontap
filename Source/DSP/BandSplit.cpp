#include "BandSplit.h"

BandSplit::BandSplit()
{
    lowpass.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    highpass.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
}

void BandSplit::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    preparedBlockSize = static_cast<int>(spec.maximumBlockSize);
    preparedChannels = static_cast<int>(spec.numChannels);

    lowpass.prepare(spec);
    highpass.prepare(spec);

    lowpass.setCutoffFrequency(cutoffFreq);
    highpass.setCutoffFrequency(cutoffFreq);

    // Allocate with extra headroom — some hosts send larger blocks than declared
    int allocSize = preparedBlockSize * 2;
    lowBand.setSize(preparedChannels, allocSize);
    highBand.setSize(preparedChannels, allocSize);
    lowBand.clear();
    highBand.clear();
}

void BandSplit::reset()
{
    lowpass.reset();
    highpass.reset();
    lowBand.clear();
    highBand.clear();
}

void BandSplit::setFrequency(float freqHz)
{
    cutoffFreq = juce::jlimit(20.0f, 20000.0f, freqHz);
    lowpass.setCutoffFrequency(cutoffFreq);
    highpass.setCutoffFrequency(cutoffFreq);
}

void BandSplit::setActive(bool active)
{
    isActive = active;
}

void BandSplit::split(juce::AudioBuffer<float>& buffer)
{
    currentNumSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Ensure our buffers are large enough
    if (lowBand.getNumSamples() < currentNumSamples || lowBand.getNumChannels() < numChannels)
    {
        lowBand.setSize(numChannels, currentNumSamples * 2, false, true);
        highBand.setSize(numChannels, currentNumSamples * 2, false, true);
    }

    // Copy input to both band buffers
    for (int ch = 0; ch < numChannels; ++ch)
    {
        lowBand.copyFrom(ch, 0, buffer, ch, 0, currentNumSamples);
        highBand.copyFrom(ch, 0, buffer, ch, 0, currentNumSamples);
    }

    // Apply crossover filters using sub-blocks of the correct size
    auto lowBlock = juce::dsp::AudioBlock<float>(lowBand).getSubBlock(0, static_cast<size_t>(currentNumSamples));
    auto highBlock = juce::dsp::AudioBlock<float>(highBand).getSubBlock(0, static_cast<size_t>(currentNumSamples));

    lowpass.process(juce::dsp::ProcessContextReplacing<float>(lowBlock));
    highpass.process(juce::dsp::ProcessContextReplacing<float>(highBlock));
}

void BandSplit::recombine(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.copyFrom(ch, 0, lowBand, ch, 0, currentNumSamples);
        buffer.addFrom(ch, 0, highBand, ch, 0, currentNumSamples);
    }
}
