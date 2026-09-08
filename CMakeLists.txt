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
# JUCE (added as subdirectory via Git submodule)
# ------------------------------
add_subdirectory(ThirdParty/JUCE)

# Set JUCE build options
set(JUCE_ENABLE_MODULE_LOGGING ON CACHE BOOL "" FORCE)
set(JUCE_BUILD_MINGW  OFF CACHE BOOL "" FORCE)

# Helper variable for JUCE modules path
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/JUCE")

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
    Source/UI/Components/OscilloscopeDisplay.cpp
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

# ============================================================
# CPack - One-Click Installer Configuration
# ============================================================
# Note: include(CPack) appears at the END of this file so that
# every CPACK_* variable below is visible when CPackConfig.cmake
# is generated. See the bottom of this file.

# Basic package information
set(CPACK_PACKAGE_NAME "GhostSignalMS20P")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_DESCRIPTION "GhostSignal MS20P - Modern Virtual Analog Synthesizer")
set(CPACK_PACKAGE_VENDOR "Ghost Signal")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/cfoltyns/GhostSignal_MS20P2")
set(CPACK_PACKAGE_LICENSE "Proprietary")

# Platform-specific installer configurations

# Windows NSIS installer
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Ghost Signal MS20P")
    set(CPACK_NSIS_PACKAGE_NAME "GhostSignalMS20P")
    set(CPACK_NSIS_MODIFY_PATH OFF)
    set(CPACK_NSIS_CREATE_UNINSTALLER ON)
    set(CPACK_NSIS_WELCOME_TEXT "Welcome to the GhostSignal MS20P installer")
    set(CPACK_NSIS_FINISH_TEXT "GhostSignal MS20P has been successfully installed")
    
    # Install to user directories to avoid admin privileges
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "GhostSignalMS20P")
    
    # VST3 install location (user VST3 folder)
    set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_SHOW_COMPLETE_PAGE ON)
    
    # Create Start Menu shortcuts
    set(CPACK_NSIS_CREATE_STARTMENU_SHORTCUT ON)
    set(CPACK_NSIS_STARTMENU_GROUP "Ghost Signal")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_PACKAGE_EXECUTABLES "GhostSignalMS20P" "GhostSignalMS20P")
    set(CPACK_CREATE_DESKTOP_LINKS "GhostSignalMS20P")
    
    # Set the installer icon if available
    if(EXISTS "${CMAKE_SOURCE_DIR}/Resources/ghostsignal.ico")
        set(CPACK_NSIS_INSTALLER_ICON "${CMAKE_SOURCE_DIR}/Resources/ghostsignal.ico")
    endif()
endif()

# macOS installer
if(APPLE)
    # Create a productbuild-based installer for macOS
    set(CPACK_GENERATOR "productbuild")
    
    # Install to Applications for standalone, user Library for AU
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "/Applications")
    
    # Set the installer title
    set(CPACK_productbuild_identifier "com.ghostsignal.GhostSignalMS20P")
    set(CPACK_productbuild_signing_identity "")

    # Include the AU component in the macOS package
    set(CPACK_COMPONENTS_ALL "App;VST3;AU;Documentation")
    set(CPACK_COMPONENT_AU_DISPLAY_NAME "Ghost Signal MS20P (AU)")
endif()

# Linux installers
if(UNIX AND NOT APPLE)
    # Generate DEB, RPM, and TGZ packages
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ghost Signal <contact@ghostsignal.com>")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.31), libstdc++6 (>= 11)")
    set(CPACK_RPM_PACKAGE_RELEASE "1")
    set(CPACK_RPM_PACKAGE_LICENSE "Proprietary")
    
    # Linux install paths
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "/opt/GhostSignalMS20P")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/GhostSignalMS20P")
    
    # Create desktop entries and menu items where applicable
    set(CPACK_RPM_PACKAGE_DESCRIPTION "GhostSignal MS20P Virtual Analog Synthesizer")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "GhostSignal MS20P Virtual Analog Synthesizer")
endif()

# Default CPack output configuration
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/License.txt")
set(CPACK_INSTALL_PROJECT_NAME "GhostSignalMS20P")

# Purge the vendored JUCE framework from the staged package.
# CPACK_PRE_BUILD_SCRIPTS runs after files are staged but before
# the package is built (see CMake/cpack_cleanup.cmake for details).
set(CPACK_PRE_BUILD_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/CMake/cpack_cleanup.cmake")

# Package ONLY the app, plugin and documentation - never the vendored
# JUCE framework files (they land in the "Unspecified" component).
set(CPACK_COMPONENTS_ALL "App;VST3;Documentation")
set(CPACK_COMPONENTS_GROUPING "ALL_COMPONENTS_IN_ONE")

set(CPACK_COMPONENT_APP_DISPLAY_NAME "Ghost Signal MS20P (Standalone)")
set(CPACK_COMPONENT_VST3_DISPLAY_NAME "Ghost Signal MS20P (VST3)")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")

# ============================================================
# End CPack Configuration
# ============================================================


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

# ------------------------------
# Install rules for CPack
# ------------------------------

# JUCE lays build artefacts out under GhostSignalMS20P_artefacts/<Config>/.
# Resolve the correct sub-folder for the active generator so the same
# install rules work for single-config (Unix Makefiles, Ninja) and
# multi-config (Visual Studio, Xcode) builds.
if(CMAKE_CONFIGURATION_TYPES)
    set(GS_ARTEFACTS_DIR "${CMAKE_BINARY_DIR}/GhostSignalMS20P_artefacts/\$<CONFIG>")
else()
    set(GS_ARTEFACTS_DIR "${CMAKE_BINARY_DIR}/GhostSignalMS20P_artefacts")
endif()

if(WIN32)
    # --- Windows: Standalone .exe + VST3 bundle ----------------------------
    install(FILES "${GS_ARTEFACTS_DIR}/Standalone/GhostSignalMS20P.exe"
        DESTINATION "App"
        COMPONENT App)
    install(DIRECTORY "${GS_ARTEFACTS_DIR}/VST3/GhostSignalMS20P.vst3"
        DESTINATION "VST3"
        COMPONENT VST3)
elseif(APPLE)
    # --- macOS: Standalone .app + VST3 bundle + AU component --------------
    install(DIRECTORY "${GS_ARTEFACTS_DIR}/Standalone/GhostSignalMS20P.app"
        DESTINATION "App"
        COMPONENT App)
    install(DIRECTORY "${GS_ARTEFACTS_DIR}/VST3/GhostSignalMS20P.vst3"
        DESTINATION "VST3"
        COMPONENT VST3)
    install(DIRECTORY "${GS_ARTEFACTS_DIR}/AU/GhostSignalMS20P.component"
        DESTINATION "AU"
        COMPONENT AU)
elseif(UNIX)
    # --- Linux: Standalone binary + VST3 bundle ---------------------------
    install(FILES "${GS_ARTEFACTS_DIR}/Standalone/GhostSignalMS20P"
        DESTINATION "bin"
        COMPONENT App)
    install(DIRECTORY "${GS_ARTEFACTS_DIR}/VST3/GhostSignalMS20P.vst3"
        DESTINATION "lib/vst3"
        COMPONENT VST3)
endif()

# Install documentation
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION "docs"
    COMPONENT Documentation)

# Install license
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/License.txt"
        DESTINATION "docs"
        COMPONENT Documentation)
endif()

# ------------------------------------------------------------
# CPack must be included LAST - CPACK_* variables are read when
# this line executes, so all of the settings above are captured.
# ------------------------------------------------------------
include(CPack)
