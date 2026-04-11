#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class CurveDisplay : public juce::Component, private juce::Timer
{
public:
    CurveDisplay(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& apvtsRef;

    // End marker position (0.0 to 1.0, default at end)
    float endMarkerPosition = 1.0f;
    bool draggingEndMarker = false;

    bool isNearEndMarker(juce::Point<float> pos) const;
    float getDisplayBoundsX(float normalizedPos) const;
    juce::Rectangle<float> getDisplayBounds() const;
};
