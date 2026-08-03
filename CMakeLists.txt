cmake_minimum_required(VERSION 3.24...3.31)
project(GhostSignalMS20P LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release build if not explicitly set by the user
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ------------------------------
# JUCE (vendored local copy)
# ------------------------------
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/JUCE")
add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

# ------------------------------
# Plugin code
# ------------------------------
juce_add_plugin(GhostSignalMS20P
    COMPANY_NAME "Ghost Signal"
    PLUGIN_MANUFACTURER_CODE Gsgn
    PLUGIN_CODE MS20
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "GhostSignalMS20P"
    VERSION "0.1.0"
    BUNDLE_ID "com.ghostsignal.ms20p"
    COPYRIGHT "(c) 2026 Ghost Signal"

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE

    # Hardened settings
    DEFAULT_PLUGIN_MAIN_CLASS_NAME PluginProcessor
    DEFAULT_EDITOR_CLASS_NAME PluginEditor
)

# Shared source alias
set(SOURCES
    Source/Plugin/PluginProcessor.cpp
    Source/Plugin/PluginEditor.cpp
    Source/Core/Parameters.cpp
    Source/Core/AudioEngine.cpp
    Source/Core/VoiceManager.cpp
    Source/Core/ModulationMatrix.cpp
    Source/Core/Arpeggiator.cpp
    Source/Core/PresetManager.cpp
    Source/Core/Sequencer.cpp
    Source/Core/PatchBay.cpp
    Source/UI/LookAndFeel.cpp
    Source/UI/Components/Panel.cpp
    Source/UI/Components/LabeledKnob.cpp
    Source/UI/Components/WaveformKnob.cpp
    Source/UI/Components/EnvDisplay.cpp
    Source/UI/Components/LfoDisplay.cpp
    Source/UI/Components/PatchPanel.cpp
    Source/UI/Components/LogoComponent.cpp
    Source/UI/Components/Analyzer.cpp
    Source/Synth/Oscillator.cpp
    Source/Synth/SubOsc.cpp
    Source/Synth/NoiseGen.cpp
    Source/Synth/Mixer.cpp
    Source/Synth/Filter/HighPass.cpp
    Source/Synth/Filter/LowPass.cpp
    Source/Synth/Filter/Saturation.cpp
    Source/Synth/Filter/Filters.cpp
    Source/Synth/Envelope.cpp
    Source/Synth/LFO.cpp
    Source/Synth/Voice.cpp
    Source/Synth/AnalogDrift.cpp
    Source/Synth/SynthEngine.cpp
    Source/Synth/Modulators.cpp
    Source/Effects/BitCrusher.cpp
    Source/Effects/Chorus.cpp
    Source/Effects/Delay.cpp
    Source/Effects/EffectsChain.cpp
    Source/Effects/TapeDelay.cpp
)

target_sources(GhostSignalMS20P PRIVATE ${SOURCES})

target_include_directories(GhostSignalMS20P PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/App
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Synth
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Synth/Filter
    ${JUCE_DIR}/modules
    ${JUCE_DIR}/modules/juce_audio_plugin_client/VST3
    ${JUCE_DIR}/modules/juce_audio_plugin_client/VST
    ${JUCE_DIR}/modules/juce_audio_plugin_client/AU
    ${JUCE_DIR}/modules/juce_audio_plugin_client/AAX
    ${JUCE_DIR}/modules/juce_audio_plugin_client/Standalone
)
juce_generate_juce_header(GhostSignalMS20P)


# ------------------------------
# Compile definitions
# ------------------------------
target_compile_definitions(GhostSignalMS20P PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

# ------------------------------
# Link JUCE modules
# ------------------------------
target_link_libraries(GhostSignalMS20P
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ------------------------------
# IDE folders
# ------------------------------
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" PREFIX "Source" FILES ${SOURCES})
