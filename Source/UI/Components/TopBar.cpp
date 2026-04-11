#include "TopBar.h"
#include "../LookAndFeel/ColorScheme.h"
#include "../../Parameters/ParameterIDs.h"

TopBar::TopBar(juce::AudioProcessorValueTreeState& apvts)
    : apvtsRef(apvts)
{
    const juce::StringArray rateNames { "1/8", "1/4", "1/2", "1/1" };
    for (int i = 0; i < 4; ++i)
    {
        rateButtons[i].setButtonText(rateNames[i]);
        rateButtons[i].setClickingTogglesState(false);
        rateButtons[i].onClick = [this, i]() {
            if (auto* param = apvtsRef.getParameter(ParameterIDs::Timing::length.getParamID()))
                param->setValueNotifyingHost(static_cast<float>(i) / 3.0f);
            updateRateSelection();
        };
        addAndMakeVisible(rateButtons[i]);
    }

    const juce::StringArray triggerNames { "SYNC", "MIDI", "AUDIO" };
    for (int i = 0; i < 3; ++i)
    {
        triggerButtons[i].setButtonText(triggerNames[i]);
        triggerButtons[i].setClickingTogglesState(false);
        triggerButtons[i].onClick = [this, i]() {
            if (auto* param = apvtsRef.getParameter(ParameterIDs::Timing::triggerMode.getParamID()))
                param->setValueNotifyingHost(static_cast<float>(i) / 2.0f);
            updateTriggerSelection();
        };
        addAndMakeVisible(triggerButtons[i]);
    }

    // No bypass button — removed per user request

    updateRateSelection();
    updateTriggerSelection();
}

void TopBar::paint(juce::Graphics& g)
{
    g.fillAll(TapOnColors::bgCard);
    g.setColour(TapOnColors::border);
    g.drawLine(0.0f, static_cast<float>(getHeight()) - 0.5f,
               static_cast<float>(getWidth()), static_cast<float>(getHeight()) - 0.5f, 1.0f);
}

void TopBar::resized()
{
    auto bounds = getLocalBounds().reduced(0, 4);
    int centreX = getWidth() / 2;

    // Rate buttons — centered, left of center
    int rateW = 48;
    int gap = 4;
    int totalRateW = rateW * 4 + gap * 3;
    int rateStartX = centreX - totalRateW - 20;
    for (int i = 0; i < 4; ++i)
        rateButtons[i].setBounds(rateStartX + i * (rateW + gap), bounds.getY(), rateW, bounds.getHeight());

    // Trigger buttons — centered, right of center
    int trigW = 60;
    int trigStartX = centreX + 20;
    for (int i = 0; i < 3; ++i)
        triggerButtons[i].setBounds(trigStartX + i * (trigW + gap), bounds.getY(), trigW, bounds.getHeight());
}

void TopBar::updateRateSelection()
{
    int current = static_cast<int>(apvtsRef.getRawParameterValue(
        ParameterIDs::Timing::length.getParamID())->load());
    for (int i = 0; i < 4; ++i)
    {
        bool isActive = (i == current);
        rateButtons[i].setColour(juce::TextButton::buttonColourId,
            isActive ? TapOnColors::orange : juce::Colours::transparentBlack);
        rateButtons[i].setColour(juce::TextButton::textColourOffId,
            isActive ? TapOnColors::bgPrimary : TapOnColors::textSecondary);
    }
}

void TopBar::updateTriggerSelection()
{
    int current = static_cast<int>(apvtsRef.getRawParameterValue(
        ParameterIDs::Timing::triggerMode.getParamID())->load());
    for (int i = 0; i < 3; ++i)
    {
        bool isActive = (i == current);
        triggerButtons[i].setColour(juce::TextButton::buttonColourId,
            isActive ? juce::Colours::transparentBlack : juce::Colours::transparentBlack);
        triggerButtons[i].setColour(juce::TextButton::textColourOffId,
            isActive ? TapOnColors::textPrimary : TapOnColors::textMuted);
        if (isActive)
            triggerButtons[i].setColour(juce::TextButton::buttonColourId,
                juce::Colours::white.withAlpha(0.05f));
    }
}
