#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters/ParameterFactory.h"

TapOnProcessor::TapOnProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                         .withInput("Sidechain", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", ParameterFactory::createParameterLayout()),
      duckingEngine(apvts)
{
#ifndef ONTAP_DEMO
    licenseManager = std::make_unique<LicenseManager>();
#endif
}

TapOnProcessor::~TapOnProcessor() = default;

bool TapOnProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainInput  = layouts.getMainInputChannelSet();
    const auto& mainOutput = layouts.getMainOutputChannelSet();

    // Only support mono or stereo, and input must match output
    if (mainOutput != juce::AudioChannelSet::mono()
        && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    if (mainInput != mainOutput)
        return false;

    return true;
}

void TapOnProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    duckingEngine.prepare(spec);
}

void TapOnProcessor::releaseResources()
{
    duckingEngine.reset();
}

void TapOnProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

#ifndef ONTAP_DEMO
    // License check — silence audio if not activated
    if (licenseManager && !licenseManager->isActivated())
    {
        buffer.clear();
        return;
    }
#endif

    double bpm = 120.0;
    double ppqPosition = 0.0;
    bool isPlaying = false;

    if (auto* playHead = getPlayHead())
    {
        auto position = playHead->getPosition();
        if (position.hasValue())
        {
            if (auto bpmOpt = position->getBpm())
                bpm = *bpmOpt;
            if (auto ppqOpt = position->getPpqPosition())
                ppqPosition = *ppqOpt;
            isPlaying = position->getIsPlaying();
        }
    }

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    duckingEngine.processBlock(buffer, midiMessages, bpm, ppqPosition, isPlaying);
}

juce::AudioProcessorEditor* TapOnProcessor::createEditor()
{
    return new TapOnEditor(*this);
}

void TapOnProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TapOnProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapOnProcessor();
}
