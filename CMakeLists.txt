cmake_minimum_required(VERSION 3.22)

project(MyAwesomePlugin VERSION 1.0.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Configure for host architecture only (not universal binary)
set(CMAKE_OSX_ARCHITECTURES ${CMAKE_HOST_SYSTEM_PROCESSOR})

# Find and add JUCE
set(JUCE_DIR "$ENV{HOME}/JUCE")
add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)

# Create the plugin target
juce_add_plugin(MyAwesomePlugin
    PLUGIN_MANUFACTURER_CODE "AWSM"
    PLUGIN_CODE "Awsm"
    FORMATS VST3 AU
    PRODUCT_NAME "Franky's Filters"
    COMPANY_NAME "Awesome Audio Co"
    BUNDLE_ID "com.awesome.myawesomeplugin"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Fx" "Filter" 
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

# Add binary data (if needed for future resources)
# juce_add_binary_data(MyAwesomePluginData
#     SOURCES
#         Resources/icon.png
# )

# Add source files
target_sources(MyAwesomePlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

# No additional third-party sources needed

# Link JUCE modules
target_link_libraries(MyAwesomePlugin PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Compiler definitions
target_compile_definitions(MyAwesomePlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_USE_COREAUDIO_DEPRECATED_API=0
    JUCE_DISABLE_AUDIO_DEVICES=1
)

# Enable testing
enable_testing()

# Add test executable
file(GLOB TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(MyAwesomePlugin_Tests 
        ${TEST_SOURCES}
        Source/PluginProcessor.cpp
    )
    target_link_libraries(MyAwesomePlugin_Tests PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_audio_formats
    )
    target_include_directories(MyAwesomePlugin_Tests PRIVATE Source)
    target_compile_definitions(MyAwesomePlugin_Tests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_UNIT_TESTS=1
        JucePlugin_Name="Franky's Filters"
        JucePlugin_Desc="Low-pass filter plugin"
        JucePlugin_Manufacturer="Awesome Audio Co"
        JucePlugin_ManufacturerWebsite=""
        JucePlugin_ManufacturerEmail=""
        JucePlugin_ManufacturerCode=0x41574553  # 'AWES'
        JucePlugin_PluginCode=0x4D414648         # 'MAFH'
        JucePlugin_IsSynth=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0
        JucePlugin_EditorRequiresKeyboardFocus=0
        JucePlugin_Version=1.0.0
        JucePlugin_VersionCode=0x10000
        JucePlugin_VersionString="1.0.0"
    )
    add_test(NAME RunTests COMMAND MyAwesomePlugin_Tests)
endif()