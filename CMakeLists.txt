cmake_minimum_required(VERSION 3.22)
project(TapOn VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

add_subdirectory(JUCE)

# --- Main Plugin ---
juce_add_plugin(TapOn
    COMPANY_NAME "Carbonated Audio"
    PLUGIN_MANUFACTURER_CODE CbAu
    PLUGIN_CODE TpOn
    FORMATS VST3 AU AAX Standalone
    PRODUCT_NAME "On Tap"
    BUNDLE_ID "com.carbonatedaudio.ontap"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx" "Dynamics"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
    AAX_CATEGORY "AAX_ePlugInCategory_Dynamics"
)

target_sources(TapOn PRIVATE
    # Core
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    # Parameters
    Source/Parameters/ParameterFactory.cpp
    # DSP
    Source/DSP/DuckingEngine.cpp
    Source/DSP/CurvePresets.cpp
    Source/DSP/BandSplit.cpp
    # Licensing
    Source/Licensing/LicenseManager.cpp
    Source/Licensing/ActivationDialog.cpp
    # UI
    Source/UI/TapOnPanel.cpp
    Source/UI/Components/TopBar.cpp
    Source/UI/Components/MixKnob.cpp
    Source/UI/Components/CurveDisplay.cpp
    Source/UI/Components/PresetGrid.cpp
    Source/UI/LookAndFeel/TapOnLookAndFeel.cpp
    Source/UI/LookAndFeel/ColorScheme.cpp
)

juce_add_binary_data(TapOnBinaryData SOURCES
    Resources/logo.png
    Resources/Inter-Bold.ttf
    Resources/Inter-Medium.ttf
    Resources/BebasNeue-Regular.ttf
)

target_link_libraries(TapOn PRIVATE TapOnBinaryData)

option(BUILD_DEMO "Build demo version with audio limitations" OFF)

target_compile_definitions(TapOn PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

if(BUILD_DEMO)
    target_compile_definitions(TapOn PUBLIC ONTAP_DEMO=1)
endif()

target_link_libraries(TapOn PRIVATE
    juce::juce_audio_utils
    juce::juce_cryptography
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if(APPLE)
    target_link_libraries(TapOn PRIVATE "-framework IOKit")
endif()
