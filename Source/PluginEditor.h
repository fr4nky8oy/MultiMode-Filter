/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>
#include "PluginProcessor.h"
#include "BinaryData.h"

// Forward declare NanoVG context
struct NVGcontext;

//==============================================================================
/**
    MyAwesome Filter Plugin Editor
*/

// NanoVG-based LookAndFeel for real-time vector graphics
class NanoVGLookAndFeel : public juce::LookAndFeel_V4
{
public:
    NanoVGLookAndFeel()
    {
        // Set basic colors (NanoVG will override the drawing)
        setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::trackColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::ToggleButton::textColourId, juce::Colours::transparentBlack);
        setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                         juce::Slider& slider) override
    {
        // NanoVG drawing will be handled in the OpenGL context
        // This method will be empty as we draw directly with NanoVG in renderOpenGL
    }
    
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, 
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // NanoVG drawing will be handled in the OpenGL context
    }
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // NanoVG drawing will be handled in the OpenGL context
    }
};

class NewPluginSkeletonAudioProcessorEditor : public juce::AudioProcessorEditor, 
                                              public juce::OpenGLRenderer,
                                              public juce::Timer,
                                              public juce::Slider::Listener,
                                              public juce::Button::Listener
{
public:
    NewPluginSkeletonAudioProcessorEditor (NewPluginSkeletonAudioProcessor&);
    ~NewPluginSkeletonAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // OpenGL methods
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
    
    // Timer callback for animations
    void timerCallback() override;
    
    // Listener callbacks
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;

private:
    NewPluginSkeletonAudioProcessor& audioProcessor;

    // NanoVG LookAndFeel
    NanoVGLookAndFeel nanoVGLookAndFeel;
    
    // OpenGL and NanoVG context
    juce::OpenGLContext openGLContext;
    NVGcontext* nvgContext = nullptr;
    int fontNormal = -1;
    int fontBold = -1;
    
    // UI Components (invisible - drawn with NanoVG)
    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;
    juce::Slider gainSlider;
    juce::ToggleButton slope6dBButton;
    juce::ToggleButton slope18dBButton;
    
    juce::ToggleButton lowPassButton;
    juce::ToggleButton highPassButton;
    juce::ToggleButton bandPassButton;
    
    // Preset management components
    juce::ComboBox presetMenu;
    juce::TextButton savePresetButton;
    juce::TextButton loadPresetButton;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    
    // Animation and interaction
    float animationTime = 0.0f;
    bool isDragging = false;
    juce::Point<int> mousePos;
    
    // Preset storage
    juce::File currentPresetDirectory;

    // NanoVG drawing methods
    void drawBackground(NVGcontext* vg, float width, float height);
    void drawRotaryKnob(NVGcontext* vg, float x, float y, float size, float value, const juce::String& label, const juce::String& valueText);
    void drawButton(NVGcontext* vg, float x, float y, float width, float height, const juce::String& text, bool isSelected, bool isPressed);
    void drawPresetSection(NVGcontext* vg, float width);
    
    // Mouse handling
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    // Preset management methods
    void savePreset();
    void loadPreset();
    void loadPresetFromDropdown();
    void updatePresetMenu();
    juce::File getCurrentPresetDirectory();
    void setCurrentPresetDirectory(const juce::File& directory);
    
    // JUCE drawing methods (fallback interface)
    void drawJuceKnob(juce::Graphics& g, int x, int y, int size, float value, 
                     const juce::String& label, const juce::String& valueText);
    void drawJuceButton(juce::Graphics& g, int x, int y, int width, int height,
                       const juce::String& text, bool isSelected);
    juce::String formatCutoffValue(float value);
    juce::String formatResonanceValue(float value);
    juce::String formatGainValue(float value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginSkeletonAudioProcessorEditor)
};
