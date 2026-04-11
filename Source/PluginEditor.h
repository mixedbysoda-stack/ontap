#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/TapOnPanel.h"
#include "UI/LookAndFeel/TapOnLookAndFeel.h"

#ifndef ONTAP_DEMO
 #include "Licensing/ActivationDialog.h"
#endif

class TapOnEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TapOnEditor(TapOnProcessor&);
    ~TapOnEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TapOnProcessor& processorRef;
    TapOnLookAndFeel tapOnLAF;
    TapOnPanel mainPanel;

#ifndef ONTAP_DEMO
    ActivationDialog activationDialog;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapOnEditor)
};
