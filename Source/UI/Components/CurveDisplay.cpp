#include "CurveDisplay.h"
#include "../LookAndFeel/ColorScheme.h"
#include "../../Parameters/ParameterIDs.h"
#include "../../DSP/CurvePresets.h"

CurveDisplay::CurveDisplay(juce::AudioProcessorValueTreeState& apvts)
    : apvtsRef(apvts)
{
    setInterceptsMouseClicks(true, false);
    startTimerHz(15);
}

juce::Rectangle<float> CurveDisplay::getDisplayBounds() const
{
    return getLocalBounds().toFloat().reduced(4.0f);
}

float CurveDisplay::getDisplayBoundsX(float normalizedPos) const
{
    auto bounds = getDisplayBounds();
    return bounds.getX() + normalizedPos * bounds.getWidth();
}

bool CurveDisplay::isNearEndMarker(juce::Point<float> pos) const
{
    float markerX = getDisplayBoundsX(endMarkerPosition);
    return std::abs(pos.x - markerX) < 10.0f;
}

void CurveDisplay::mouseDown(const juce::MouseEvent& e)
{
    if (isNearEndMarker(e.position))
    {
        draggingEndMarker = true;
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    }
}

void CurveDisplay::mouseDrag(const juce::MouseEvent& e)
{
    if (draggingEndMarker)
    {
        auto bounds = getDisplayBounds();
        float newPos = (e.position.x - bounds.getX()) / bounds.getWidth();
        endMarkerPosition = juce::jlimit(0.05f, 1.0f, newPos);
        repaint();
    }
}

void CurveDisplay::mouseUp(const juce::MouseEvent&)
{
    draggingEndMarker = false;
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void CurveDisplay::mouseMove(const juce::MouseEvent& e)
{
    if (isNearEndMarker(e.position))
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    else
        setMouseCursor(juce::MouseCursor::NormalCursor);
}

void CurveDisplay::paint(juce::Graphics& g)
{
    auto bounds = getDisplayBounds();

    // Background
    g.setColour(TapOnColors::bgDarker);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Grid lines
    g.setColour(juce::Colours::white.withAlpha(0.03f));
    for (int i = 1; i < 4; ++i)
    {
        float y = bounds.getY() + bounds.getHeight() * static_cast<float>(i) / 4.0f;
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
    }
    for (int i = 1; i < 8; ++i)
    {
        float x = bounds.getX() + bounds.getWidth() * static_cast<float>(i) / 8.0f;
        g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
    }

    // Get parameters
    int presetIndex = static_cast<int>(apvtsRef.getRawParameterValue(
        ParameterIDs::Curve::presetIndex.getParamID())->load());
    float slopeOffset = apvtsRef.getRawParameterValue(
        ParameterIDs::Curve::slopeOffset.getParamID())->load();
    float shift = apvtsRef.getRawParameterValue(
        ParameterIDs::Curve::shiftAmount.getParamID())->load();

    CurvePresets presets;
    float w = bounds.getWidth();
    float h = bounds.getHeight();

    // Dimmed region after end marker
    if (endMarkerPosition < 0.99f)
    {
        float endX = bounds.getX() + endMarkerPosition * w;
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRect(endX, bounds.getY(), bounds.getRight() - endX, h);
    }

    // Simulated waveform (white, low opacity) — ducked by curve
    {
        juce::Path wavePath;
        juce::Path mirrorPath;
        for (int i = 0; i <= static_cast<int>(w); ++i)
        {
            float x = static_cast<float>(i) / w;
            float waveVal = std::sin(x * juce::MathConstants<float>::pi * 18.0f) * 0.25f +
                            std::sin(x * juce::MathConstants<float>::pi * 7.3f) * 0.18f +
                            std::sin(x * juce::MathConstants<float>::pi * 31.0f) * 0.06f;

            // If past end marker, use the curve value at the end marker position
            float curveX = (x <= endMarkerPosition) ? x : endMarkerPosition;
            float curveVal = presets.getValue(presetIndex, curveX, slopeOffset, shift);
            float amplitude = 0.3f + curveVal * 0.5f;
            float plotX = bounds.getX() + static_cast<float>(i);

            float plotY = bounds.getCentreY() + waveVal * amplitude * h * 0.45f;
            if (i == 0) wavePath.startNewSubPath(plotX, plotY);
            else wavePath.lineTo(plotX, plotY);

            float mirrorY = bounds.getCentreY() - waveVal * amplitude * h * 0.45f;
            if (i == 0) mirrorPath.startNewSubPath(plotX, mirrorY);
            else mirrorPath.lineTo(plotX, mirrorY);
        }
        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.strokePath(wavePath, juce::PathStrokeType(1.0f));
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.strokePath(mirrorPath, juce::PathStrokeType(1.0f));
    }

    // Main ducking curve (orange)
    {
        juce::Path curvePath;
        for (int i = 0; i <= static_cast<int>(w); ++i)
        {
            float x = static_cast<float>(i) / w;
            float curveX = (x <= endMarkerPosition) ? x : endMarkerPosition;
            float curveVal = presets.getValue(presetIndex, curveX, slopeOffset, shift);
            float plotX = bounds.getX() + static_cast<float>(i);
            float plotY = bounds.getBottom() - curveVal * (h - 20.0f) - 10.0f;

            if (i == 0) curvePath.startNewSubPath(plotX, plotY);
            else curvePath.lineTo(plotX, plotY);
        }
        g.setColour(TapOnColors::orange);
        g.strokePath(curvePath, juce::PathStrokeType(2.5f));
    }

    // End marker line
    {
        float markerX = bounds.getX() + endMarkerPosition * w;
        g.setColour(juce::Colours::white.withAlpha(draggingEndMarker ? 0.7f : 0.4f));
        g.drawVerticalLine(static_cast<int>(markerX), bounds.getY(), bounds.getBottom());

        // Handle at top
        float handleW = 10.0f;
        float handleH = 16.0f;
        g.setColour(juce::Colours::white.withAlpha(draggingEndMarker ? 0.8f : 0.5f));
        g.fillRoundedRectangle(markerX - handleW / 2.0f, bounds.getY() + 4.0f, handleW, handleH, 2.0f);

        // Grip lines on handle
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.drawVerticalLine(static_cast<int>(markerX - 2), bounds.getY() + 8.0f, bounds.getY() + 16.0f);
        g.drawVerticalLine(static_cast<int>(markerX),     bounds.getY() + 8.0f, bounds.getY() + 16.0f);
        g.drawVerticalLine(static_cast<int>(markerX + 2), bounds.getY() + 8.0f, bounds.getY() + 16.0f);
    }
}

void CurveDisplay::resized() {}

void CurveDisplay::timerCallback()
{
    repaint();
}
