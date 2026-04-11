#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/DuckingEngine.h"

#ifndef ONTAP_DEMO
 #include "Licensing/LicenseManager.h"
#endif

class TapOnProcessor : public juce::AudioProcessor
{
public:
    TapOnProcessor();
    ~TapOnProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

#ifndef ONTAP_DEMO
    LicenseManager& getLicenseManager() { return *licenseManager; }
#endif

private:
    juce::AudioProcessorValueTreeState apvts;
    DuckingEngine duckingEngine;

#ifndef ONTAP_DEMO
    std::unique_ptr<LicenseManager> licenseManager;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapOnProcessor)
};
