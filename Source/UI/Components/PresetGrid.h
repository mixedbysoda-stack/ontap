#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class PresetGrid : public juce::Component
{
public:
    PresetGrid(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& apvtsRef;
    int getPresetIndexAtPosition(juce::Point<int> pos) const;
};
