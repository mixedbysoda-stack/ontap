#include "ParameterFactory.h"
#include "ParameterIDs.h"

juce::AudioProcessorValueTreeState::ParameterLayout ParameterFactory::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Mix (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::Mix::mix, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Curve preset index (0-15)
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        ParameterIDs::Curve::presetIndex, "Curve Preset", 0, 15, 4));

    // Slope offset (-1.0 to 1.0, how much the slope portion is shifted)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::Curve::slopeOffset, "Slope Offset",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f));

    // Shift amount (-1.0 to 1.0, shifts entire curve left/right)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::Curve::shiftAmount, "Shift",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), 0.0f));

    // Length (1/8, 1/4, 1/2, 1/1)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParameterIDs::Timing::length, "Length",
        juce::StringArray { "1/8", "1/4", "1/2", "1/1" }, 1));

    // Trigger mode (Sync, MIDI, Audio)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParameterIDs::Timing::triggerMode, "Trigger Mode",
        juce::StringArray { "Sync", "MIDI", "Audio" }, 0));

    // Loop on/off
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::Timing::loop, "Loop", true));

    // Band split active
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::Band::active, "Band Active", false));

    // Band split frequency (20Hz - 5120Hz, log scale)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::Band::frequency, "Band Frequency",
        juce::NormalisableRange<float>(20.0f, 5120.0f, 1.0f, 0.3f), 320.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::Global::bypass, "Bypass", false));

    // Anti-click smoothing
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::Global::antiClick, "Anti-Click Smoothing", true));

    return { params.begin(), params.end() };
}
