/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewPluginSkeletonAudioProcessorEditor::NewPluginSkeletonAudioProcessorEditor (NewPluginSkeletonAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set custom look and feel
    setLookAndFeel(&modernLookAndFeel);
    
    // Configure main window
    setSize(500, 400);
    setResizable(true, true);
    setResizeLimits(400, 300, 800, 600);
    
    // Title label
    titleLabel.setText("MyAwesome Filter", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Cutoff slider
    cutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutoffSlider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(cutoffSlider);
    
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffLabel);
    
    cutoffValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffValueLabel);
    
    // Resonance slider
    resonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    resonanceSlider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(resonanceSlider);
    
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resonanceLabel);
    
    resonanceValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resonanceValueLabel);
    
    // Gain slider
    gainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setPopupDisplayEnabled(true, true, this);
    addAndMakeVisible(gainSlider);
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);
    
    gainValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainValueLabel);
    
    // Slope buttons
    slopeLabel.setText("Slope", juce::dontSendNotification);
    slopeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(slopeLabel);
    
    slope6dBButton.setButtonText("6 dB/oct");
    slope6dBButton.setRadioGroupId(1);
    slope6dBButton.setClickingTogglesState(true);
    addAndMakeVisible(slope6dBButton);
    
    slope12dBButton.setButtonText("12 dB/oct");
    slope12dBButton.setRadioGroupId(1);
    slope12dBButton.setClickingTogglesState(true);
    addAndMakeVisible(slope12dBButton);
    
    slope24dBButton.setButtonText("24 dB/oct");
    slope24dBButton.setRadioGroupId(1);
    slope24dBButton.setClickingTogglesState(true);
    addAndMakeVisible(slope24dBButton);
    
    // Filter type buttons
    filterTypeLabel.setText("Filter Type", juce::dontSendNotification);
    filterTypeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterTypeLabel);
    
    lowPassButton.setButtonText("Low Pass");
    lowPassButton.setRadioGroupId(2);
    lowPassButton.setClickingTogglesState(true);
    addAndMakeVisible(lowPassButton);
    
    highPassButton.setButtonText("High Pass");
    highPassButton.setRadioGroupId(2);
    highPassButton.setClickingTogglesState(true);
    addAndMakeVisible(highPassButton);
    
    bandPassButton.setButtonText("Band Pass");
    bandPassButton.setRadioGroupId(2);
    bandPassButton.setClickingTogglesState(true);
    addAndMakeVisible(bandPassButton);
    
    // Create parameter attachments
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "cutoff", cutoffSlider);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "resonance", resonanceSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "gain", gainSlider);
    
    // Set initial button states based on parameters
    auto* slopeParam = audioProcessor.parameters.getParameter("slope");
    if (slopeParam)
    {
        int slopeIndex = static_cast<int>(slopeParam->getValue() * 2.0f);
        switch (slopeIndex)
        {
            case 0: slope6dBButton.setToggleState(true, juce::dontSendNotification); break;
            case 1: slope12dBButton.setToggleState(true, juce::dontSendNotification); break;
            case 2: slope24dBButton.setToggleState(true, juce::dontSendNotification); break;
        }
    }
    
    auto* filterTypeParam = audioProcessor.parameters.getParameter("filterType");
    if (filterTypeParam)
    {
        int typeIndex = static_cast<int>(filterTypeParam->getValue() * 2.0f);
        switch (typeIndex)
        {
            case 0: lowPassButton.setToggleState(true, juce::dontSendNotification); break;
            case 1: highPassButton.setToggleState(true, juce::dontSendNotification); break;
            case 2: bandPassButton.setToggleState(true, juce::dontSendNotification); break;
        }
    }
    
    // Add button listeners
    slope6dBButton.onClick = [this]() {
        if (slope6dBButton.getToggleState())
            audioProcessor.parameters.getParameter("slope")->setValueNotifyingHost(0.0f);
    };
    
    slope12dBButton.onClick = [this]() {
        if (slope12dBButton.getToggleState())
            audioProcessor.parameters.getParameter("slope")->setValueNotifyingHost(0.5f);
    };
    
    slope24dBButton.onClick = [this]() {
        if (slope24dBButton.getToggleState())
            audioProcessor.parameters.getParameter("slope")->setValueNotifyingHost(1.0f);
    };
    
    lowPassButton.onClick = [this]() {
        if (lowPassButton.getToggleState())
            audioProcessor.parameters.getParameter("filterType")->setValueNotifyingHost(0.0f);
    };
    
    highPassButton.onClick = [this]() {
        if (highPassButton.getToggleState())
            audioProcessor.parameters.getParameter("filterType")->setValueNotifyingHost(0.5f);
    };
    
    bandPassButton.onClick = [this]() {
        if (bandPassButton.getToggleState())
            audioProcessor.parameters.getParameter("filterType")->setValueNotifyingHost(1.0f);
    };
    
    // Start timer for value label updates
    startTimerHz(30);
    updateValueLabels();
}

NewPluginSkeletonAudioProcessorEditor::~NewPluginSkeletonAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

//==============================================================================
void NewPluginSkeletonAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient backgroundGradient(juce::Colour(0xff1e2a3a), 0, 0,
                                           juce::Colour(0xff2c3e50), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(backgroundGradient);
    g.fillAll();
    
    // Draw section dividers
    g.setColour(juce::Colour(0xff34495e));
    const int sectionY = getHeight() * 0.55f;
    g.drawLine(20, sectionY, getWidth() - 20, sectionY, 1.0f);
}

void NewPluginSkeletonAudioProcessorEditor::resized()
{
    const int margin = 20;
    const int titleHeight = 40;
    const int knobSize = 100;
    const int labelHeight = 20;
    const int buttonHeight = 30;
    const int buttonWidth = 80;
    
    // Title
    titleLabel.setBounds(margin, margin, getWidth() - margin * 2, titleHeight);
    
    // Calculate knob positions
    const int knobY = margin + titleHeight + 20;
    const int knobSpacing = (getWidth() - margin * 2 - knobSize * 3) / 2;
    
    // Cutoff
    int xPos = margin;
    cutoffLabel.setBounds(xPos, knobY - labelHeight, knobSize, labelHeight);
    cutoffSlider.setBounds(xPos, knobY, knobSize, knobSize);
    cutoffValueLabel.setBounds(xPos, knobY + knobSize, knobSize, labelHeight);
    
    // Resonance
    xPos += knobSize + knobSpacing;
    resonanceLabel.setBounds(xPos, knobY - labelHeight, knobSize, labelHeight);
    resonanceSlider.setBounds(xPos, knobY, knobSize, knobSize);
    resonanceValueLabel.setBounds(xPos, knobY + knobSize, knobSize, labelHeight);
    
    // Gain
    xPos += knobSize + knobSpacing;
    gainLabel.setBounds(xPos, knobY - labelHeight, knobSize, labelHeight);
    gainSlider.setBounds(xPos, knobY, knobSize, knobSize);
    gainValueLabel.setBounds(xPos, knobY + knobSize, knobSize, labelHeight);
    
    // Button sections
    const int buttonSectionY = knobY + knobSize + labelHeight + 30;
    
    // Slope buttons
    slopeLabel.setBounds(margin, buttonSectionY, 100, labelHeight);
    slope6dBButton.setBounds(margin, buttonSectionY + labelHeight + 5, buttonWidth, buttonHeight);
    slope12dBButton.setBounds(margin + buttonWidth + 10, buttonSectionY + labelHeight + 5, buttonWidth, buttonHeight);
    slope24dBButton.setBounds(margin + (buttonWidth + 10) * 2, buttonSectionY + labelHeight + 5, buttonWidth, buttonHeight);
    
    // Filter type buttons
    const int filterTypeX = getWidth() / 2 + 20;
    filterTypeLabel.setBounds(filterTypeX, buttonSectionY, 100, labelHeight);
    lowPassButton.setBounds(filterTypeX, buttonSectionY + labelHeight + 5, buttonWidth, buttonHeight);
    highPassButton.setBounds(filterTypeX + buttonWidth + 10, buttonSectionY + labelHeight + 5, buttonWidth, buttonHeight);
    bandPassButton.setBounds(filterTypeX + (buttonWidth + 10) * 2, buttonSectionY + labelHeight + 5, buttonWidth, buttonHeight);
}

void NewPluginSkeletonAudioProcessorEditor::timerCallback()
{
    updateValueLabels();
}

void NewPluginSkeletonAudioProcessorEditor::updateValueLabels()
{
    cutoffValueLabel.setText(formatCutoffValue(cutoffSlider.getValue()), juce::dontSendNotification);
    resonanceValueLabel.setText(formatResonanceValue(resonanceSlider.getValue()), juce::dontSendNotification);
    gainValueLabel.setText(formatGainValue(gainSlider.getValue()), juce::dontSendNotification);
}

juce::String NewPluginSkeletonAudioProcessorEditor::formatCutoffValue(float value)
{
    if (value < 1000.0f)
        return juce::String(value, 0) + " Hz";
    else
        return juce::String(value / 1000.0f, 2) + " kHz";
}

juce::String NewPluginSkeletonAudioProcessorEditor::formatResonanceValue(float value)
{
    return "Q: " + juce::String(value, 2);
}

juce::String NewPluginSkeletonAudioProcessorEditor::formatGainValue(float value)
{
    return juce::String(value, 1) + " dB";
}