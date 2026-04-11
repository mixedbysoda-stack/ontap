#pragma once
#include <cmath>
#include <array>
#include <functional>

class CurvePresets
{
public:
    static constexpr int numPresets = 16;
    static constexpr int tableSize = 1024;

    CurvePresets();

    // Get the ducking value (0.0 = full duck, 1.0 = no duck) for a given
    // normalized position (0.0 to 1.0) in the curve, with optional slope offset and shift
    float getValue(int presetIndex, float position, float slopeOffset = 0.0f, float shift = 0.0f) const;

    // Get the raw lookup table for a preset (for UI drawing)
    const std::array<float, tableSize>& getTable(int presetIndex) const;

private:
    void generateTables();

    // Each preset is a lookup table of 1024 samples
    std::array<std::array<float, tableSize>, numPresets> tables;
};
