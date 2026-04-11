#pragma once
#include <juce_core/juce_core.h>

namespace ParameterIDs
{
    static constexpr int paramVersion = 1;

    namespace Mix
    {
        const juce::ParameterID mix { "mix", paramVersion };
    }

    namespace Curve
    {
        const juce::ParameterID presetIndex { "curvePreset", paramVersion };
        const juce::ParameterID slopeOffset { "slopeOffset", paramVersion };
        const juce::ParameterID shiftAmount { "shiftAmount", paramVersion };
    }

    namespace Timing
    {
        const juce::ParameterID length { "length", paramVersion };          // 0=1/8, 1=1/4, 2=1/2, 3=1/1
        const juce::ParameterID triggerMode { "triggerMode", paramVersion }; // 0=Sync, 1=MIDI, 2=Audio
        const juce::ParameterID loop { "loop", paramVersion };
    }

    namespace Band
    {
        const juce::ParameterID active { "bandActive", paramVersion };
        const juce::ParameterID frequency { "bandFreq", paramVersion };
    }

    namespace Global
    {
        const juce::ParameterID bypass { "bypass", paramVersion };
        const juce::ParameterID antiClick { "antiClick", paramVersion };
    }

    enum class LengthValue { Eighth = 0, Quarter, Half, Whole };
    enum class TriggerMode { Sync = 0, MIDI, Audio };
}
