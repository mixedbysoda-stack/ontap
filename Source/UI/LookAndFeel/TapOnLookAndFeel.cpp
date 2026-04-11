#include "TapOnLookAndFeel.h"
#include "ColorScheme.h"

TapOnLookAndFeel::TapOnLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, TapOnColors::orange);
    setColour(juce::Slider::rotarySliderOutlineColourId, TapOnColors::bgCard);
    setColour(juce::Slider::thumbColourId, TapOnColors::orange);
}

void TapOnLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                         juce::Slider&)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(8.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Knob body
    g.setColour(TapOnColors::bgCard);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Outer ring shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    // Arc track (background)
    juce::Path arcTrack;
    arcTrack.addCentredArc(centreX, centreY, radius + 6.0f, radius + 6.0f,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.strokePath(arcTrack, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Arc value
    if (sliderPos > 0.01f)
    {
        juce::Path arcValue;
        arcValue.addCentredArc(centreX, centreY, radius + 6.0f, radius + 6.0f,
                               0.0f, rotaryStartAngle, angle, true);
        g.setColour(TapOnColors::orange);
        g.strokePath(arcValue, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Position indicator dot
    auto dotRadius = 5.0f;
    auto dotDistance = radius * 0.7f;
    auto dotX = centreX + dotDistance * std::cos(angle - juce::MathConstants<float>::halfPi);
    auto dotY = centreY + dotDistance * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.setColour(TapOnColors::orange);
    g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
}

void TapOnLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float, float,
                                         juce::Slider::SliderStyle, juce::Slider&)
{
    auto trackHeight = 10.0f;
    auto trackY = y + (height - trackHeight) / 2.0f;

    // Track background
    g.setColour(TapOnColors::bgPrimary);
    g.fillRoundedRectangle(static_cast<float>(x), trackY, static_cast<float>(width), trackHeight, 5.0f);

    // Filled portion
    float fillWidth = sliderPos - x;
    if (fillWidth > 0.0f)
    {
        g.setColour(TapOnColors::orange);
        g.fillRoundedRectangle(static_cast<float>(x), trackY, fillWidth, trackHeight, 5.0f);
    }
}
