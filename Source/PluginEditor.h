/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//==============================================================================
/**
    MyAwesome Filter Plugin Editor - Using JUCE Native Graphics
*/

// Custom LookAndFeel for modern flat design with subtle 3D effects
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel()
    {
        // Set color scheme
        setColour(juce::Slider::thumbColourId, juce::Colour(0xff4a90e2));
        setColour(juce::Slider::trackColourId, juce::Colour(0xff2c3e50));
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xff34495e));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4a90e2));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2c3e50));
        
        setColour(juce::ToggleButton::textColourId, juce::Colour(0xffecf0f1));
        setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff4a90e2));
        setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff7f8c8d));
        
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2c3e50));
        setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff4a90e2));
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xffecf0f1));
        setColour(juce::TextButton::textColourOnId, juce::Colour(0xffffffff));
        
        setColour(juce::Label::textColourId, juce::Colour(0xffecf0f1));
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2c3e50));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffecf0f1));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff34495e));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                         juce::Slider& slider) override
    {
        const float radius = juce::jmin(width / 2, height / 2) - 8.0f;
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // Background circle with subtle gradient
        juce::ColourGradient gradient(juce::Colour(0xff34495e), centreX, centreY - radius,
                                      juce::Colour(0xff2c3e50), centreX, centreY + radius, false);
        g.setGradientFill(gradient);
        g.fillEllipse(rx, ry, rw, rw);
        
        // Outer ring
        g.setColour(juce::Colour(0xff1a252f));
        g.drawEllipse(rx, ry, rw, rw, 2.0f);
        
        // Value arc
        juce::Path valuePath;
        valuePath.addPieSegment(rx, ry, rw, rw, rotaryStartAngle, angle, 0.85f);
        g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
        g.fillPath(valuePath);
        
        // Center knob with 3D effect
        const float knobRadius = radius * 0.6f;
        juce::ColourGradient knobGradient(juce::Colour(0xff4a90e2), centreX - knobRadius * 0.5f, centreY - knobRadius * 0.5f,
                                          juce::Colour(0xff2c5aa0), centreX + knobRadius * 0.5f, centreY + knobRadius * 0.5f, false);
        g.setGradientFill(knobGradient);
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
        
        // Pointer line
        juce::Path pointer;
        const float pointerLength = knobRadius * 0.8f;
        const float pointerThickness = 3.0f;
        pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength * 0.7f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillPath(pointer);
        
        // Inner shadow for depth
        g.setColour(juce::Colour(0x30000000));
        g.drawEllipse(centreX - knobRadius + 1, centreY - knobRadius + 1, 
                     knobRadius * 2.0f - 2, knobRadius * 2.0f - 2, 1.0f);
    }
    
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, 
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        const auto bounds = button.getLocalBounds().toFloat();
        const bool isOn = button.getToggleState();
        
        // Background with gradient
        juce::ColourGradient bgGradient(isOn ? juce::Colour(0xff4a90e2) : juce::Colour(0xff34495e), 
                                        bounds.getX(), bounds.getY(),
                                        isOn ? juce::Colour(0xff3a7bc8) : juce::Colour(0xff2c3e50), 
                                        bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(bgGradient);
        g.fillRoundedRectangle(bounds, 4.0f);
        
        // Border
        g.setColour(isOn ? juce::Colour(0xff5ca0f2) : juce::Colour(0xff1a252f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);
        
        // Text
        g.setColour(isOn ? juce::Colours::white : juce::Colour(0xffbdc3c7));
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
        
        // Highlight on hover
        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour(juce::Colours::white.withAlpha(0.1f));
            g.fillRoundedRectangle(bounds, 4.0f);
        }
    }
};

class NewPluginSkeletonAudioProcessorEditor : public juce::AudioProcessorEditor,
                                              public juce::Timer
{
public:
    NewPluginSkeletonAudioProcessorEditor (NewPluginSkeletonAudioProcessor&);
    ~NewPluginSkeletonAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback for meter updates
    void timerCallback() override;

private:
    NewPluginSkeletonAudioProcessor& audioProcessor;

    // Custom LookAndFeel
    ModernLookAndFeel modernLookAndFeel;
    
    // UI Components
    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;
    juce::Slider gainSlider;
    
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label gainLabel;
    
    juce::Label cutoffValueLabel;
    juce::Label resonanceValueLabel;
    juce::Label gainValueLabel;
    
    juce::ToggleButton slope6dBButton;
    juce::ToggleButton slope12dBButton;
    juce::ToggleButton slope24dBButton;
    
    juce::ToggleButton lowPassButton;
    juce::ToggleButton highPassButton;
    juce::ToggleButton bandPassButton;
    
    juce::Label titleLabel;
    juce::Label slopeLabel;
    juce::Label filterTypeLabel;
    
    // Preset management components
    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton loadPresetButton;
    
    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> slopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> filterTypeAttachment;
    
    // Helper methods
    void updateValueLabels();
    juce::String formatCutoffValue(float value);
    juce::String formatResonanceValue(float value);
    juce::String formatGainValue(float value);
    
    // Preset management methods
    void savePreset();
    void loadPreset();
    void updatePresetComboBox();
    juce::File getPresetDirectory();
    void createPresetDirectory();
    juce::String getCurrentPresetName();
    void setCurrentPresetName(const juce::String& name);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginSkeletonAudioProcessorEditor)
};