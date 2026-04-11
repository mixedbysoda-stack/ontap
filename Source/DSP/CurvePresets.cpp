#include "CurvePresets.h"
#include <algorithm>

CurvePresets::CurvePresets()
{
    generateTables();
}

float CurvePresets::getValue(int presetIndex, float position, float slopeOffset, float shift) const
{
    presetIndex = std::clamp(presetIndex, 0, numPresets - 1);

    // Apply shift
    position += shift;
    position = std::fmod(position + 10.0f, 1.0f); // wrap around

    // Apply slope offset: stretch/compress the sloped portion
    if (std::abs(slopeOffset) > 0.001f)
    {
        // Remap position through a power curve to adjust slope timing
        float power = std::pow(2.0f, -slopeOffset * 2.0f);
        position = std::pow(position, power);
    }

    // Lookup with linear interpolation
    float indexF = position * (tableSize - 1);
    int idx0 = static_cast<int>(indexF);
    int idx1 = std::min(idx0 + 1, tableSize - 1);
    float frac = indexF - idx0;

    return tables[presetIndex][idx0] * (1.0f - frac) + tables[presetIndex][idx1] * frac;
}

const std::array<float, CurvePresets::tableSize>& CurvePresets::getTable(int presetIndex) const
{
    return tables[std::clamp(presetIndex, 0, numPresets - 1)];
}

void CurvePresets::generateTables()
{
    auto fillTable = [this](int index, std::function<float(float)> fn) {
        for (int i = 0; i < tableSize; ++i)
        {
            float x = static_cast<float>(i) / (tableSize - 1);
            tables[index][i] = std::clamp(fn(x), 0.0f, 1.0f);
        }
    };

    // Sidechaining curves (0-7)
    // 0: Sharp kick duck
    fillTable(0, [](float x) { return x < 0.05f ? 1.0f : std::exp(-x * 8.0f) * 0.9f; });
    // 1: Medium duck
    fillTable(1, [](float x) { return std::exp(-x * 4.0f); });
    // 2: Slow duck
    fillTable(2, [](float x) { return std::exp(-x * 2.0f); });
    // 3: Linear ramp down
    fillTable(3, [](float x) { return 1.0f - x; });
    // 4: S-curve (classic sidechain)
    fillTable(4, [](float x) { return 1.0f - 1.0f / (1.0f + std::exp(-12.0f * (x - 0.5f))); });
    // 5: Punchy
    fillTable(5, [](float x) { return x < 0.1f ? 1.0f - x * 5.0f : 0.5f * std::exp(-(x - 0.1f) * 6.0f); });
    // 6: Half duck
    fillTable(6, [](float x) { return x < 0.5f ? std::exp(-x * 6.0f) : 0.0f; });
    // 7: Smooth cosine
    fillTable(7, [](float x) { return (1.0f + std::cos(x * 3.14159265f)) / 2.0f; });

    // Sub bass sidechaining (8-9)
    // 8: Double pump
    fillTable(8, [](float x) { return std::abs(std::sin(x * 3.14159265f * 2.0f)) * std::exp(-x * 3.0f); });
    // 9: Sub bass smooth
    fillTable(9, [](float x) { return x; }); // reverse ramp

    // Kick trimming (10-11)
    // 10: Stepped
    fillTable(10, [](float x) { return x < 0.33f ? 1.0f : (x < 0.66f ? 0.5f : 0.0f); });
    // 11: Gentle decay
    fillTable(11, [](float x) { return std::exp(-x * 1.5f); });

    // Reverse chain (12-13)
    // 12: Aggressive
    fillTable(12, [](float x) { return x < 0.02f ? 1.0f : std::exp(-x * 16.0f) * 0.7f; });
    // 13: Wobble
    fillTable(13, [](float x) { return std::exp(-x * 3.0f) * (0.5f + 0.5f * std::cos(x * 3.14159265f * 6.0f)); });

    // Rhythm FX (14-15)
    // 14: Flat top decay
    fillTable(14, [](float x) { return x < 0.3f ? 1.0f : std::exp(-(x - 0.3f) * 5.0f); });
    // 15: Quick release
    fillTable(15, [](float x) { return x < 0.15f ? 1.0f - x * 6.0f : std::max(0.0f, 0.1f - (x - 0.15f) * 0.15f); });
}
