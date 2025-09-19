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
    
    // Configure main window - fixed size
    setSize(600, 450);
    setResizable(false, false);
    
    // Title label
    titleLabel.setText("Franky's Filters", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
    
    // Preset management components
    presetComboBox.setTextWhenNoChoicesAvailable("No Presets");
    presetComboBox.setTextWhenNothingSelected("Select Preset...");
    addAndMakeVisible(presetComboBox);
    
    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this]() { savePreset(); };
    addAndMakeVisible(savePresetButton);
    
    loadPresetButton.setButtonText("Load");
    loadPresetButton.onClick = [this]() { loadPreset(); };
    addAndMakeVisible(loadPresetButton);
    
    presetComboBox.onChange = [this]() {
        if (presetComboBox.getSelectedItemIndex() >= 0)
        {
            auto presetName = presetComboBox.getItemText(presetComboBox.getSelectedItemIndex());
            auto presetFile = getPresetDirectory().getChildFile(presetName + ".xml");
            if (presetFile.existsAsFile())
            {
                auto xml = juce::parseXML(presetFile);
                if (xml != nullptr)
                {
                    auto valueTree = juce::ValueTree::fromXml(*xml);
                    audioProcessor.parameters.replaceState(valueTree);
                    updateButtonStates(); // Update button states after loading preset
                }
            }
        }
    };
    
    // Create preset directory and update combo box
    createPresetDirectory();
    updatePresetComboBox();
    
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
    
    // Start timer for value label updates and button state sync
    startTimerHz(30);
    updateValueLabels();
    updateButtonStates();
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
    
    // Draw section divider line (separates rotary controls from filter controls)
    g.setColour(juce::Colour(0xff4a5568));
    const int dividerY = 260; // Match the position from resized()
    g.drawLine(20, dividerY, getWidth() - 20, dividerY, 2.0f);
    
    // Add subtle shadow below the line for depth
    g.setColour(juce::Colour(0x30000000));
    g.drawLine(20, dividerY + 1, getWidth() - 20, dividerY + 1, 1.0f);
}

void NewPluginSkeletonAudioProcessorEditor::resized()
{
    const int margin = 20;
    const int titleHeight = 40;
    const int presetHeight = 25;
    const int presetButtonWidth = 60;
    const int knobSize = 100;
    const int labelHeight = 20;
    const int buttonHeight = 30;
    const int buttonWidth = 85;
    
    // Fixed dimensions for 600x450 window
    const int totalWidth = 600;
    const int totalHeight = 450;
    const int dividerY = 260; // Position of the dividing line - moved up a bit for better balance
    
    // === TOP SECTION: Title and Preset Controls ===
    
    // Title label
    const int titleWidth = 200;
    titleLabel.setBounds(margin, margin, titleWidth, titleHeight);
    
    // Preset controls on the right side of title
    const int presetStartX = margin + titleWidth + 20;
    const int presetComboWidth = 150;
    presetComboBox.setBounds(presetStartX, margin + 8, presetComboWidth, presetHeight);
    savePresetButton.setBounds(presetStartX + presetComboWidth + 10, margin + 8, presetButtonWidth, presetHeight);
    loadPresetButton.setBounds(presetStartX + presetComboWidth + presetButtonWidth + 20, margin + 8, presetButtonWidth, presetHeight);
    
    // === ROTARY CONTROLS SECTION (Above divider line) ===
    
    const int knobSectionTop = margin + titleHeight + 20;
    const int knobY = knobSectionTop + labelHeight + 5;
    const int availableWidth = totalWidth - (margin * 2);
    
    // Center the three knobs horizontally
    const int totalKnobWidth = knobSize * 3;
    const int knobSpacing = 60; // Fixed spacing between knobs
    const int totalWidthNeeded = totalKnobWidth + (knobSpacing * 2);
    const int startX = (totalWidth - totalWidthNeeded) / 2;
    
    // Cutoff
    int xPos = startX;
    cutoffLabel.setBounds(xPos, knobSectionTop, knobSize, labelHeight);
    cutoffSlider.setBounds(xPos, knobY, knobSize, knobSize);
    cutoffValueLabel.setBounds(xPos, knobY + knobSize + 5, knobSize, labelHeight);
    
    // Resonance
    xPos = startX + knobSize + knobSpacing;
    resonanceLabel.setBounds(xPos, knobSectionTop, knobSize, labelHeight);
    resonanceSlider.setBounds(xPos, knobY, knobSize, knobSize);
    resonanceValueLabel.setBounds(xPos, knobY + knobSize + 5, knobSize, labelHeight);
    
    // Gain
    xPos = startX + (knobSize + knobSpacing) * 2;
    gainLabel.setBounds(xPos, knobSectionTop, knobSize, labelHeight);
    gainSlider.setBounds(xPos, knobY, knobSize, knobSize);
    gainValueLabel.setBounds(xPos, knobY + knobSize + 5, knobSize, labelHeight);
    
    // === BUTTON CONTROLS SECTION (Below divider line) ===
    
    const int bottomSectionY = dividerY + 15; // Start just below the divider
    const int buttonSectionHeight = totalHeight - bottomSectionY - margin;
    
    // Center both button groups vertically in the bottom section
    const int buttonGroupHeight = labelHeight + 10 + buttonHeight;
    const int buttonGroupY = bottomSectionY + (buttonSectionHeight - buttonGroupHeight) / 2;
    
    // Calculate horizontal positions for centered button groups
    const int slopeGroupWidth = buttonWidth * 3 + 10;
    const int filterGroupWidth = buttonWidth * 3 + 10;
    const int totalButtonWidth = slopeGroupWidth + filterGroupWidth;
    const int buttonGroupSpacing = 40;
    const int buttonStartX = (totalWidth - totalButtonWidth - buttonGroupSpacing) / 2;
    
    // Slope section (left group)
    const int slopeLabelX = buttonStartX;
    slopeLabel.setBounds(slopeLabelX, buttonGroupY, slopeGroupWidth, labelHeight);
    const int slopeButtonY = buttonGroupY + labelHeight + 10;
    slope6dBButton.setBounds(slopeLabelX, slopeButtonY, buttonWidth, buttonHeight);
    slope12dBButton.setBounds(slopeLabelX + buttonWidth + 5, slopeButtonY, buttonWidth, buttonHeight);
    slope24dBButton.setBounds(slopeLabelX + (buttonWidth + 5) * 2, slopeButtonY, buttonWidth, buttonHeight);
    
    // Filter type section (right group)
    const int filterTypeX = buttonStartX + slopeGroupWidth + buttonGroupSpacing;
    filterTypeLabel.setBounds(filterTypeX, buttonGroupY, filterGroupWidth, labelHeight);
    const int filterButtonY = buttonGroupY + labelHeight + 10;
    lowPassButton.setBounds(filterTypeX, filterButtonY, buttonWidth, buttonHeight);
    highPassButton.setBounds(filterTypeX + buttonWidth + 5, filterButtonY, buttonWidth, buttonHeight);
    bandPassButton.setBounds(filterTypeX + (buttonWidth + 5) * 2, filterButtonY, buttonWidth, buttonHeight);
}

void NewPluginSkeletonAudioProcessorEditor::timerCallback()
{
    updateValueLabels();
    updateButtonStates();
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

void NewPluginSkeletonAudioProcessorEditor::updateButtonStates()
{
    // Update slope buttons based on parameter value
    auto* slopeParam = audioProcessor.parameters.getParameter("slope");
    if (slopeParam)
    {
        float slopeValue = slopeParam->getValue();
        int slopeIndex = static_cast<int>(slopeValue * 2.0f + 0.5f); // Round to nearest int
        
        slope6dBButton.setToggleState(slopeIndex == 0, juce::dontSendNotification);
        slope12dBButton.setToggleState(slopeIndex == 1, juce::dontSendNotification);
        slope24dBButton.setToggleState(slopeIndex == 2, juce::dontSendNotification);
    }
    
    // Update filter type buttons based on parameter value
    auto* filterTypeParam = audioProcessor.parameters.getParameter("filterType");
    if (filterTypeParam)
    {
        float typeValue = filterTypeParam->getValue();
        int typeIndex = static_cast<int>(typeValue * 2.0f + 0.5f); // Round to nearest int
        
        lowPassButton.setToggleState(typeIndex == 0, juce::dontSendNotification);
        highPassButton.setToggleState(typeIndex == 1, juce::dontSendNotification);
        bandPassButton.setToggleState(typeIndex == 2, juce::dontSendNotification);
    }
}

void NewPluginSkeletonAudioProcessorEditor::savePreset()
{
    showPresetNameDialog([this](const juce::String& presetName) {
        if (presetName.isNotEmpty())
        {
            auto presetFile = getPresetDirectory().getChildFile(presetName + ".xml");
            
            // Get current state from processor
            auto state = audioProcessor.parameters.copyState();
            auto xml = state.createXml();
            
            if (xml != nullptr && xml->writeTo(presetFile))
            {
                updatePresetComboBox();
                
                // Select the newly saved preset
                for (int i = 0; i < presetComboBox.getNumItems(); ++i)
                {
                    if (presetComboBox.getItemText(i) == presetName)
                    {
                        presetComboBox.setSelectedItemIndex(i, juce::dontSendNotification);
                        break;
                    }
                }
            }
        }
    });
}

void NewPluginSkeletonAudioProcessorEditor::loadPreset()
{
    auto chooser = std::make_unique<juce::FileChooser>("Load Preset", getPresetDirectory(), "*.xml");
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this](const juce::FileChooser& fc)
                        {
                            auto file = fc.getResult();
                            if (file != juce::File{})
                            {
                                auto xml = juce::parseXML(file);
                                
                                if (xml != nullptr)
                                {
                                    auto valueTree = juce::ValueTree::fromXml(*xml);
                                    audioProcessor.parameters.replaceState(valueTree);
                                    updateButtonStates(); // Update button states after loading
                                    
                                    // Update combo box selection
                                    auto presetName = file.getFileNameWithoutExtension();
                                    for (int i = 0; i < presetComboBox.getNumItems(); ++i)
                                    {
                                        if (presetComboBox.getItemText(i) == presetName)
                                        {
                                            presetComboBox.setSelectedItemIndex(i, juce::dontSendNotification);
                                            break;
                                        }
                                    }
                                }
                            }
                        });
}

void NewPluginSkeletonAudioProcessorEditor::updatePresetComboBox()
{
    presetComboBox.clear(juce::dontSendNotification);
    
    auto presetDir = getPresetDirectory();
    if (presetDir.exists())
    {
        juce::Array<juce::File> presetFiles;
        presetDir.findChildFiles(presetFiles, juce::File::findFiles, false, "*.xml");
        
        for (const auto& file : presetFiles)
        {
            presetComboBox.addItem(file.getFileNameWithoutExtension(), presetComboBox.getNumItems() + 1);
        }
    }
}

juce::File NewPluginSkeletonAudioProcessorEditor::getPresetDirectory()
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    
#if JUCE_MAC
    return appDataDir.getChildFile("Audio/Presets/Franky's Filters");
#elif JUCE_WINDOWS  
    return appDataDir.getChildFile("Franky's Filters/Presets");
#else
    return appDataDir.getChildFile(".FrankysFilters/Presets");
#endif
}

void NewPluginSkeletonAudioProcessorEditor::createPresetDirectory()
{
    auto presetDir = getPresetDirectory();
    if (!presetDir.exists())
    {
        presetDir.createDirectory();
    }
}

juce::String NewPluginSkeletonAudioProcessorEditor::getCurrentPresetName()
{
    auto selectedIndex = presetComboBox.getSelectedItemIndex();
    if (selectedIndex >= 0)
        return presetComboBox.getItemText(selectedIndex);
    return juce::String();
}

void NewPluginSkeletonAudioProcessorEditor::setCurrentPresetName(const juce::String& name)
{
    for (int i = 0; i < presetComboBox.getNumItems(); ++i)
    {
        if (presetComboBox.getItemText(i) == name)
        {
            presetComboBox.setSelectedItemIndex(i, juce::dontSendNotification);
            break;
        }
    }
}

void NewPluginSkeletonAudioProcessorEditor::showPresetNameDialog(std::function<void(const juce::String&)> callback)
{
    // Create a simple input dialog
    class PresetNameDialog : public juce::Component
    {
    public:
        PresetNameDialog(std::function<void(const juce::String&)> onComplete)
            : onCompleteCallback(onComplete)
        {
            addAndMakeVisible(nameEditor);
            addAndMakeVisible(saveButton);
            addAndMakeVisible(cancelButton);
            
            nameEditor.setText("New Preset");
            nameEditor.selectAll();
            
            saveButton.setButtonText("Save");
            cancelButton.setButtonText("Cancel");
            
            saveButton.onClick = [this]() {
                if (onCompleteCallback)
                    onCompleteCallback(nameEditor.getText());
                closeDialog();
            };
            
            cancelButton.onClick = [this]() {
                closeDialog();
            };
            
            setSize(300, 120);
            nameEditor.onReturnKey = [this]() { saveButton.onClick(); };
        }
        
        void resized() override
        {
            auto area = getLocalBounds().reduced(20);
            nameEditor.setBounds(area.removeFromTop(30));
            area.removeFromTop(10);
            
            auto buttonArea = area.removeFromTop(30);
            saveButton.setBounds(buttonArea.removeFromLeft(80));
            buttonArea.removeFromLeft(10);
            cancelButton.setBounds(buttonArea.removeFromLeft(80));
        }
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xff2c3e50));
            g.setColour(juce::Colours::white);
            g.setFont(16.0f);
            g.drawText("Enter preset name:", 20, 10, getWidth() - 40, 20, juce::Justification::centredLeft);
        }
        
        void parentHierarchyChanged() override
        {
            if (auto* parent = getParentComponent())
            {
                centreWithSize(getWidth(), getHeight());
                nameEditor.grabKeyboardFocus();
            }
        }
        
    private:
        juce::TextEditor nameEditor;
        juce::TextButton saveButton, cancelButton;
        std::function<void(const juce::String&)> onCompleteCallback;
        
        void closeDialog()
        {
            if (auto* parent = getParentComponent())
                parent->removeChildComponent(this);
            delete this;
        }
    };
    
    auto* dialog = new PresetNameDialog(callback);
    addAndMakeVisible(dialog);
    dialog->toFront(true);
}