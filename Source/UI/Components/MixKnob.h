#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MixKnob : public juce::Component
{
public:
    MixKnob(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void updateBandState();

    // Fonts
    juce::Font displayFont;  // Bebas Neue for "ON TAP"
    juce::Font labelFont;    // Inter Bold for "CARBONATED AUDIO"
    juce::Font readoutFont;  // Inter Medium for readouts

    // Mix knob
    juce::Slider mixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // Band split controls
    juce::TextButton bandButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bandAttachment;
    juce::Slider freqSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freqAttachment;
};
