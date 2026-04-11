#include "PresetGrid.h"
#include "../LookAndFeel/ColorScheme.h"
#include "../../Parameters/ParameterIDs.h"
#include "../../DSP/CurvePresets.h"

PresetGrid::PresetGrid(juce::AudioProcessorValueTreeState& apvts)
    : apvtsRef(apvts)
{
    setInterceptsMouseClicks(true, false);
}

void PresetGrid::paint(juce::Graphics& g)
{
    int activePreset = static_cast<int>(apvtsRef.getRawParameterValue(
        ParameterIDs::Curve::presetIndex.getParamID())->load());

    CurvePresets presets;
    int cols = 8;
    int rows = 2;
    float cellW = static_cast<float>(getWidth()) / static_cast<float>(cols);
    float cellH = static_cast<float>(getHeight()) / static_cast<float>(rows);

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            int index = row * cols + col;
            float x = col * cellW;
            float y = row * cellH;
            auto cellBounds = juce::Rectangle<float>(x, y, cellW, cellH).reduced(2.0f);

            bool isActive = (index == activePreset);

            // Cell background
            g.setColour(isActive ? TapOnColors::orange.withAlpha(0.08f) : TapOnColors::bgCard);
            g.fillRoundedRectangle(cellBounds, 4.0f);

            // Cell border
            g.setColour(isActive ? TapOnColors::orange : juce::Colours::white.withAlpha(0.06f));
            g.drawRoundedRectangle(cellBounds, 4.0f, isActive ? 2.0f : 1.0f);

            // Draw mini curve
            juce::Path miniPath;
            float cw = cellBounds.getWidth() - 6.0f;
            float ch = cellBounds.getHeight() - 6.0f;
            float cx = cellBounds.getX() + 3.0f;
            float cy = cellBounds.getY() + 3.0f;

            for (int i = 0; i <= static_cast<int>(cw); ++i)
            {
                float pos = static_cast<float>(i) / cw;
                float val = presets.getValue(index, pos);
                float px = cx + static_cast<float>(i);
                float py = cy + ch - val * ch;

                if (i == 0)
                    miniPath.startNewSubPath(px, py);
                else
                    miniPath.lineTo(px, py);
            }

            g.setColour(isActive ? TapOnColors::orange : juce::Colours::white.withAlpha(0.25f));
            g.strokePath(miniPath, juce::PathStrokeType(isActive ? 2.0f : 1.5f));
        }
    }
}

void PresetGrid::resized() {}

void PresetGrid::mouseDown(const juce::MouseEvent& e)
{
    int index = getPresetIndexAtPosition(e.getPosition());
    if (index >= 0 && index < CurvePresets::numPresets)
    {
        if (auto* param = apvtsRef.getParameter(ParameterIDs::Curve::presetIndex.getParamID()))
            param->setValueNotifyingHost(static_cast<float>(index) / static_cast<float>(CurvePresets::numPresets - 1));
        repaint();
    }
}

int PresetGrid::getPresetIndexAtPosition(juce::Point<int> pos) const
{
    if (!getLocalBounds().contains(pos))
        return -1;

    int cols = 8;
    int rows = 2;
    int col = pos.x * cols / juce::jmax(1, getWidth());
    int row = pos.y * rows / juce::jmax(1, getHeight());
    col = juce::jlimit(0, cols - 1, col);
    row = juce::jlimit(0, rows - 1, row);
    return row * cols + col;
}
