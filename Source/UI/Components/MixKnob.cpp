#include "MixKnob.h"
#include "../LookAndFeel/ColorScheme.h"
#include "../../Parameters/ParameterIDs.h"
#include "BinaryData.h"

MixKnob::MixKnob(juce::AudioProcessorValueTreeState& apvts)
{
    setInterceptsMouseClicks(true, true);

    // Load custom fonts from binary data with null safety
    auto bebasTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::BebasNeueRegular_ttf, BinaryData::BebasNeueRegular_ttfSize);
    auto interBoldTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterBold_ttf, BinaryData::InterBold_ttfSize);

    if (bebasTypeface != nullptr)
        displayFont = juce::Font(juce::FontOptions(bebasTypeface).withHeight(34.0f));
    else
        displayFont = juce::Font(juce::FontOptions(34.0f));

    if (interBoldTypeface != nullptr)
    {
        labelFont = juce::Font(juce::FontOptions(interBoldTypeface).withHeight(10.0f));
        readoutFont = juce::Font(juce::FontOptions(interBoldTypeface).withHeight(15.0f));
    }
    else
    {
        labelFont = juce::Font(juce::FontOptions(10.0f));
        readoutFont = juce::Font(juce::FontOptions(15.0f));
    }

    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixSlider.setColour(juce::Slider::rotarySliderFillColourId, TapOnColors::orange);
    mixSlider.setColour(juce::Slider::rotarySliderOutlineColourId, TapOnColors::bgCard);
    mixSlider.setPopupDisplayEnabled(false, false, nullptr);
    addAndMakeVisible(mixSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::Mix::mix.getParamID(), mixSlider);

    // Band controls
    bandButton.setButtonText("BAND");
    bandButton.setClickingTogglesState(true);
    bandButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    bandButton.setColour(juce::TextButton::buttonOnColourId, TapOnColors::orange);
    bandButton.setColour(juce::TextButton::textColourOffId, TapOnColors::textSecondary);
    bandButton.setColour(juce::TextButton::textColourOnId, TapOnColors::bgPrimary);
    bandButton.onClick = [this]() {
        updateBandState();
    };
    addAndMakeVisible(bandButton);
    bandAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParameterIDs::Band::active.getParamID(), bandButton);

    freqSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    freqSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    freqSlider.setColour(juce::Slider::trackColourId, TapOnColors::orange);
    freqSlider.setColour(juce::Slider::backgroundColourId, TapOnColors::bgPrimary);
    freqSlider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
    freqSlider.setPopupDisplayEnabled(false, false, nullptr);
    freqSlider.onValueChange = [this]() { repaint(); };
    addAndMakeVisible(freqSlider);
    freqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParameterIDs::Band::frequency.getParamID(), freqSlider);

    // Set initial state — slider disabled until band is on
    updateBandState();
}

void MixKnob::updateBandState()
{
    bool bandActive = bandButton.getToggleState();
    freqSlider.setEnabled(bandActive);
    freqSlider.setAlpha(bandActive ? 1.0f : 0.3f);
    freqSlider.setInterceptsMouseClicks(bandActive, bandActive);
    repaint();
}

static juce::String formatFrequency(float freqHz)
{
    if (freqHz >= 1000.0f)
        return juce::String(freqHz / 1000.0f, 1) + "kHz";
    return juce::String(static_cast<int>(freqHz)) + "Hz";
}

void MixKnob::paint(juce::Graphics& g)
{
    g.fillAll(TapOnColors::bgSecondary);

    // Border on right side
    g.setColour(TapOnColors::border);
    g.drawVerticalLine(getWidth() - 1, 0.0f, static_cast<float>(getHeight()));

    // Branding — top left
    g.setColour(TapOnColors::textMuted);
    g.setFont(labelFont);
    g.drawText("CARBONATED AUDIO", 16, 10, 200, 12, juce::Justification::left);

    g.setColour(TapOnColors::textPrimary);
    g.setFont(displayFont);
    g.drawText("ON TAP", 16, 22, 200, 34, juce::Justification::left);

    // Mix readout — below knob
    auto value = static_cast<int>(mixSlider.getValue());
    g.setColour(TapOnColors::textPrimary);
    g.setFont(readoutFont);

    auto knobBounds = mixSlider.getBounds();
    g.drawText("MIX " + juce::String(value) + "%",
               knobBounds.getX(), knobBounds.getBottom() + 4, knobBounds.getWidth(), 20,
               juce::Justification::centred);

    // --- Band section details ---
    auto bandBounds = bandButton.getBounds();

    // Headphones icon next to BAND
    float iconX = static_cast<float>(bandBounds.getRight() + 10);
    float iconY = static_cast<float>(bandBounds.getCentreY()) - 7.0f;
    g.setColour(TapOnColors::textMuted);
    // Headphone band (arc path)
    juce::Path headphoneArc;
    headphoneArc.addCentredArc(iconX + 7.0f, iconY + 7.0f, 7.0f, 7.0f,
                                0.0f,
                                -juce::MathConstants<float>::pi,
                                0.0f, true);
    g.strokePath(headphoneArc, juce::PathStrokeType(1.5f));
    // Left ear
    g.fillRoundedRectangle(iconX, iconY + 6.0f, 4.0f, 6.0f, 1.5f);
    // Right ear
    g.fillRoundedRectangle(iconX + 10.0f, iconY + 6.0f, 4.0f, 6.0f, 1.5f);

    // Frequency readout — right side of band row
    if (bandButton.getToggleState())
    {
        float freq = static_cast<float>(freqSlider.getValue());
        g.setColour(TapOnColors::orange);
        g.setFont(labelFont.withHeight(11.0f));
        g.drawText(formatFrequency(freq),
                   getWidth() - 80, bandBounds.getY(), 64, bandBounds.getHeight(),
                   juce::Justification::centredRight);
    }

    // Frequency slider labels
    auto sliderBounds = freqSlider.getBounds();
    bool bandActive = bandButton.getToggleState();
    float labelAlpha = bandActive ? 1.0f : 0.3f;

    g.setFont(labelFont.withHeight(8.0f));
    g.setColour(TapOnColors::textMuted.withAlpha(labelAlpha));
    g.drawText("20Hz", sliderBounds.getX(), sliderBounds.getBottom() + 2,
               40, 12, juce::Justification::left);
    g.drawText("5.1kHz", sliderBounds.getRight() - 40, sliderBounds.getBottom() + 2,
               40, 12, juce::Justification::right);

    // Current frequency in center below slider
    if (bandActive)
    {
        float freq = static_cast<float>(freqSlider.getValue());
        // Position the label at the slider thumb position
        float normPos = static_cast<float>((freqSlider.getValue() - freqSlider.getMinimum()) /
                        (freqSlider.getMaximum() - freqSlider.getMinimum()));
        float labelX = sliderBounds.getX() + normPos * sliderBounds.getWidth();

        g.setColour(TapOnColors::orange);
        g.setFont(labelFont.withHeight(10.0f));
        g.drawText(formatFrequency(freq),
                   static_cast<int>(labelX) - 25, sliderBounds.getBottom() + 2,
                   50, 12, juce::Justification::centred);
    }
}

void MixKnob::resized()
{
    auto bounds = getLocalBounds();

    // Knob — centered in the middle area
    int knobSize = 140;
    int knobY = 70;
    int knobX = (bounds.getWidth() - knobSize) / 2;
    mixSlider.setBounds(knobX, knobY, knobSize, knobSize);

    // Bottom section: BAND button + frequency slider
    int bottomY = bounds.getHeight() - 60;

    bandButton.setBounds(16, bottomY, 60, 24);
    freqSlider.setBounds(16, bottomY + 28, bounds.getWidth() - 32, 12);
}
