#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class TopBar : public juce::Component
{
public:
    TopBar(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::TextButton rateButtons[4];
    juce::TextButton triggerButtons[3];

    juce::AudioProcessorValueTreeState& apvtsRef;

    void updateRateSelection();
    void updateTriggerSelection();
};
