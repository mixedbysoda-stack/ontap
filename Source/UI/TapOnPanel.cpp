#include "TapOnPanel.h"
#include "LookAndFeel/ColorScheme.h"
#include "BinaryData.h"

TapOnPanel::TapOnPanel(juce::AudioProcessorValueTreeState& apvts)
    : topBar(apvts), mixKnob(apvts), curveDisplay(apvts), presetGrid(apvts)
{
    addAndMakeVisible(curveDisplay);
    addAndMakeVisible(presetGrid);
    addAndMakeVisible(topBar);
    addAndMakeVisible(mixKnob);

    // Load the logo from binary data
    logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
}

void TapOnPanel::paint(juce::Graphics& g)
{
    g.fillAll(TapOnColors::bgSecondary);

    // Draw logo in the navigation row between curve display and preset grid
    // This row sits at the right side, between the two sections
    int logoRowY = getHeight() - 120 - 28; // above preset grid, below curve display
    int logoRowX = 280 + 8; // after left panel
    int logoRowW = getWidth() - 280 - 16;

    // Draw the logo centered in the nav row
    if (logoImage.isValid())
    {
        // Scale logo to fit nicely — about 120px wide, maintaining aspect ratio
        float logoDisplayW = 120.0f;
        float aspect = static_cast<float>(logoImage.getWidth()) / static_cast<float>(logoImage.getHeight());
        float logoDisplayH = logoDisplayW / aspect;

        // Cap height to fit in the row
        if (logoDisplayH > 22.0f)
        {
            logoDisplayH = 22.0f;
            logoDisplayW = logoDisplayH * aspect;
        }

        float logoX = logoRowX + (logoRowW - logoDisplayW) / 2.0f;
        float logoY = static_cast<float>(logoRowY) + (28.0f - logoDisplayH) / 2.0f;

        g.setOpacity(0.6f);
        g.drawImage(logoImage,
                    logoX, logoY, logoDisplayW, logoDisplayH,
                    0, 0, logoImage.getWidth(), logoImage.getHeight());
        g.setOpacity(1.0f);
    }
}

void TapOnPanel::resized()
{
    auto bounds = getLocalBounds();

    // Top bar: 42px high, full width
    topBar.setBounds(bounds.removeFromTop(42));

    // Left panel: 280px wide
    auto leftPanel = bounds.removeFromLeft(280);
    mixKnob.setBounds(leftPanel);

    // Right side bottom: preset grid (120px tall)
    auto presetArea = bounds.removeFromBottom(120);
    presetGrid.setBounds(presetArea.reduced(4));

    // Navigation row (28px) — logo is painted here, not a separate component
    bounds.removeFromBottom(28);

    // Right side remaining: curve display
    curveDisplay.setBounds(bounds.reduced(4));
}
