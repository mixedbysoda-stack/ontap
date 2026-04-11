#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Components/TopBar.h"
#include "Components/MixKnob.h"
#include "Components/CurveDisplay.h"
#include "Components/PresetGrid.h"

class TapOnPanel : public juce::Component
{
public:
    TapOnPanel(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    TopBar topBar;
    MixKnob mixKnob;
    CurveDisplay curveDisplay;
    PresetGrid presetGrid;

    juce::Image logoImage;
};
