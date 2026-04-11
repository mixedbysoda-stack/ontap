#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class ParameterFactory
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
};
