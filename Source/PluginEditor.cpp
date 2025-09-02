/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

// Define OpenGL macros to use JUCE's wrapped OpenGL functions
using namespace juce::gl;
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"

//==============================================================================
NewPluginSkeletonAudioProcessorEditor::NewPluginSkeletonAudioProcessorEditor (NewPluginSkeletonAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Skip OpenGL setup - use pure JUCE interface
    juce::Logger::writeToLog("Using professional JUCE interface (OpenGL disabled)");
    nvgContext = nullptr;  // Ensure we use JUCE fallback
    
    // Set NanoVG LookAndFeel
    setLookAndFeel(&nanoVGLookAndFeel);
    
    // Configure sliders (invisible for custom drawing)
    cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutoffSlider.setAlpha(0.0f); // Keep invisible for custom drawing
    cutoffSlider.addListener(this);
    addAndMakeVisible(cutoffSlider);
    
    resonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    resonanceSlider.setAlpha(0.0f);
    resonanceSlider.addListener(this);
    addAndMakeVisible(resonanceSlider);
    
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setAlpha(0.0f);
    gainSlider.addListener(this);
    addAndMakeVisible(gainSlider);
    
    // Configure buttons (invisible for custom drawing)
    slope6dBButton.setButtonText("6 dB/oct");
    slope6dBButton.setRadioGroupId(1);
    slope6dBButton.setToggleState(true, juce::dontSendNotification);
    slope6dBButton.setAlpha(0.0f);
    slope6dBButton.addListener(this);
    addAndMakeVisible(slope6dBButton);
    
    slope18dBButton.setButtonText("18 dB/oct");
    slope18dBButton.setRadioGroupId(1);
    slope18dBButton.setAlpha(0.0f);
    slope18dBButton.addListener(this);
    addAndMakeVisible(slope18dBButton);
    
    lowPassButton.setButtonText("Low-pass");
    lowPassButton.setRadioGroupId(2);
    lowPassButton.setToggleState(true, juce::dontSendNotification);
    lowPassButton.setAlpha(0.0f);
    lowPassButton.addListener(this);
    addAndMakeVisible(lowPassButton);
    
    highPassButton.setButtonText("High-pass");
    highPassButton.setRadioGroupId(2);
    highPassButton.setAlpha(0.0f);
    highPassButton.addListener(this);
    addAndMakeVisible(highPassButton);
    
    bandPassButton.setButtonText("Band-pass");
    bandPassButton.setRadioGroupId(2);
    bandPassButton.setAlpha(0.0f);
    bandPassButton.addListener(this);
    addAndMakeVisible(bandPassButton);
    
    // Configure preset components (invisible)
    presetMenu.setAlpha(0.0f);
    addAndMakeVisible(presetMenu);
    
    savePresetButton.setButtonText("Save");
    savePresetButton.setAlpha(0.0f);
    savePresetButton.onClick = [this]() { savePreset(); };
    addAndMakeVisible(savePresetButton);
    
    loadPresetButton.setButtonText("Load");
    loadPresetButton.setAlpha(0.0f);
    loadPresetButton.onClick = [this]() { loadPreset(); };
    addAndMakeVisible(loadPresetButton);
    
    // Button callbacks for parameter changes
    slope6dBButton.onClick = [this]()
    {
        if (slope6dBButton.getToggleState())
            if (auto* param = audioProcessor.parameters.getParameter("slope"))
                param->setValueNotifyingHost(0.0f);
    };
    
    slope18dBButton.onClick = [this]()
    {
        if (slope18dBButton.getToggleState())
            if (auto* param = audioProcessor.parameters.getParameter("slope"))
                param->setValueNotifyingHost(1.0f);
    };
    
    lowPassButton.onClick = [this]()
    {
        if (lowPassButton.getToggleState())
            if (auto* param = audioProcessor.parameters.getParameter("filterType"))
                param->setValueNotifyingHost(0.0f);
    };
    
    highPassButton.onClick = [this]()
    {
        if (highPassButton.getToggleState())
            if (auto* param = audioProcessor.parameters.getParameter("filterType"))
                param->setValueNotifyingHost(0.5f);
    };
    
    bandPassButton.onClick = [this]()
    {
        if (bandPassButton.getToggleState())
            if (auto* param = audioProcessor.parameters.getParameter("filterType"))
                param->setValueNotifyingHost(1.0f);
    };
    
    // Create parameter attachments
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "cutoff", cutoffSlider);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "resonance", resonanceSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "gain", gainSlider);
    
    // Initialize preset directory
    currentPresetDirectory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                           .getChildFile("Franky Filter Presets");
    if (!currentPresetDirectory.exists())
        currentPresetDirectory.createDirectory();
    
    updatePresetMenu();
    
    // Start animation timer for smooth effects
    startTimerHz(60); // 60 FPS
    
    setSize(600, 560);
}

NewPluginSkeletonAudioProcessorEditor::~NewPluginSkeletonAudioProcessorEditor()
{
    openGLContext.detach();
    setLookAndFeel(nullptr);
}

//==============================================================================
void NewPluginSkeletonAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Professional JUCE interface (NanoVG fallback)
    if (nvgContext == nullptr)
    {
        // Modern gradient background
        juce::ColourGradient gradient(juce::Colour(40, 45, 85), 0, 0,
                                     juce::Colour(25, 30, 55), 0, getHeight(), false);
        g.setGradientFill(gradient);
        g.fillAll();
        
        // Title
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font("Arial", 28.0f, juce::Font::bold));
        g.drawText("Franky Filter", getLocalBounds().removeFromTop(80), juce::Justification::centred);
        
        // Preset section background
        auto presetArea = juce::Rectangle<int>(20, 10, getWidth() - 40, 50);
        g.setColour(juce::Colour(50, 55, 75).withAlpha(0.8f));
        g.fillRoundedRectangle(presetArea.toFloat(), 6.0f);
        g.setColour(juce::Colour(80, 85, 105));
        g.drawRoundedRectangle(presetArea.toFloat(), 6.0f, 1.0f);
        
        // Draw professional knobs
        drawJuceKnob(g, 100, 120, 120, cutoffSlider.getValue() / cutoffSlider.getMaximum(), 
                    "Cutoff", formatCutoffValue(cutoffSlider.getValue()));
        drawJuceKnob(g, 275, 120, 120, resonanceSlider.getValue() / resonanceSlider.getMaximum(),
                    "Resonance", formatResonanceValue(resonanceSlider.getValue()));
        drawJuceKnob(g, 450, 120, 120, (gainSlider.getValue() - gainSlider.getMinimum()) / 
                    (gainSlider.getMaximum() - gainSlider.getMinimum()),
                    "Gain", formatGainValue(gainSlider.getValue()));
        
        // Draw buttons
        drawJuceButton(g, getWidth()/2 - 130, 300, 120, 35, "6 dB/oct", slope6dBButton.getToggleState());
        drawJuceButton(g, getWidth()/2 + 10, 300, 120, 35, "18 dB/oct", slope18dBButton.getToggleState());
        
        drawJuceButton(g, getWidth()/2 - 175, 360, 110, 35, "Low-pass", lowPassButton.getToggleState());
        drawJuceButton(g, getWidth()/2 - 55, 360, 110, 35, "High-pass", highPassButton.getToggleState());
        drawJuceButton(g, getWidth()/2 + 65, 360, 110, 35, "Band-pass", bandPassButton.getToggleState());
        
        return;
    }
    
    // OpenGL/NanoVG handles all drawing
    g.fillAll(juce::Colours::black);
}

void NewPluginSkeletonAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Position invisible components for mouse interaction
    auto presetSection = bounds.removeFromTop(50);
    presetSection.reduce(20, 10);
    
    auto buttonWidth = 60;
    auto menuWidth = presetSection.getWidth() - (buttonWidth * 2 + 20);
    
    presetMenu.setBounds(presetSection.removeFromLeft(menuWidth));
    presetSection.removeFromLeft(10);
    savePresetButton.setBounds(presetSection.removeFromLeft(buttonWidth));
    presetSection.removeFromLeft(10);
    loadPresetButton.setBounds(presetSection);
    
    bounds.removeFromTop(50); // Title space
    bounds.reduce(30, 20);
    
    // Position knobs for interaction
    auto topSection = bounds.removeFromTop(200);
    auto sliderWidth = (topSection.getWidth() - 40) / 3;
    
    cutoffSlider.setBounds(topSection.removeFromLeft(sliderWidth).removeFromTop(150));
    topSection.removeFromLeft(20);
    resonanceSlider.setBounds(topSection.removeFromLeft(sliderWidth).removeFromTop(150));
    topSection.removeFromLeft(20);
    gainSlider.setBounds(topSection.removeFromTop(150));
    
    bounds.removeFromTop(30);
    
    // Position slope buttons
    auto slopeButtonArea = bounds.removeFromTop(80);
    slopeButtonArea.removeFromTop(30); // Label space
    auto slopeButtonWidth = 120;
    auto slopeStartX = (slopeButtonArea.getWidth() - (slopeButtonWidth * 2 + 20)) / 2;
    
    slope6dBButton.setBounds(slopeStartX, slopeButtonArea.getY(), slopeButtonWidth, 40);
    slope18dBButton.setBounds(slopeStartX + slopeButtonWidth + 20, slopeButtonArea.getY(), slopeButtonWidth, 40);
    
    bounds.removeFromTop(20);
    
    // Position filter type buttons
    auto typeButtonArea = bounds.removeFromTop(80);
    typeButtonArea.removeFromTop(30); // Label space
    auto typeButtonWidth = 110;
    auto typeStartX = (typeButtonArea.getWidth() - (typeButtonWidth * 3 + 40)) / 2;
    
    lowPassButton.setBounds(typeStartX, typeButtonArea.getY(), typeButtonWidth, 40);
    highPassButton.setBounds(typeStartX + typeButtonWidth + 20, typeButtonArea.getY(), typeButtonWidth, 40);
    bandPassButton.setBounds(typeStartX + (typeButtonWidth + 20) * 2, typeButtonArea.getY(), typeButtonWidth, 40);
}

//==============================================================================
// OpenGL Methods

void NewPluginSkeletonAudioProcessorEditor::newOpenGLContextCreated()
{
    juce::Logger::writeToLog("OpenGL context created, initializing NanoVG...");
    
    // Initialize NanoVG with OpenGL3
    nvgContext = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    
    if (nvgContext == nullptr)
    {
        juce::Logger::writeToLog("ERROR: Failed to create NanoVG context");
        return;
    }
    
    juce::Logger::writeToLog("SUCCESS: NanoVG context created");
    
    // Load fonts from binary data
    fontNormal = nvgCreateFontMem(nvgContext, "roboto-regular", 
                                 (unsigned char*)BinaryData::RobotoRegular_ttf,
                                 BinaryData::RobotoRegular_ttfSize, 0);
    
    fontBold = nvgCreateFontMem(nvgContext, "roboto-bold",
                               (unsigned char*)BinaryData::RobotoBold_ttf, 
                               BinaryData::RobotoBold_ttfSize, 0);
                               
    juce::Logger::writeToLog("Fonts loaded - Normal: " + juce::String(fontNormal) + " Bold: " + juce::String(fontBold));
}

void NewPluginSkeletonAudioProcessorEditor::renderOpenGL()
{
    if (nvgContext == nullptr)
    {
        juce::Logger::writeToLog("WARNING: NanoVG context is null in renderOpenGL");
        return;
    }
        
    auto bounds = getLocalBounds();
    auto width = (float)bounds.getWidth();
    auto height = (float)bounds.getHeight();
    
    // Begin NanoVG frame
    nvgBeginFrame(nvgContext, width, height, 1.0f);
    
    // Draw animated background
    drawBackground(nvgContext, width, height);
    
    // Draw preset section
    drawPresetSection(nvgContext, width);
    
    // Draw simple text
    if (fontBold != -1)
    {
        nvgFontFaceId(nvgContext, fontBold);
        nvgFontSize(nvgContext, 24.0f);
        nvgFillColor(nvgContext, nvgRGBA(255, 255, 0, 255));
        nvgTextAlign(nvgContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(nvgContext, width * 0.5f, 85.0f, "Franky Filter", nullptr);
    }
    
    // Draw rotary knobs
    float knobY = 170.0f;
    float knobSize = 120.0f;
    float knobSpacing = (width - 90.0f) / 3.0f;
    
    // Get current parameter values
    float cutoffValue = cutoffSlider.getValue();
    float resonanceValue = resonanceSlider.getValue();
    float gainValue = gainSlider.getValue();
    
    // Format value strings
    juce::String cutoffText = cutoffValue >= 1000.0f ? 
        juce::String((int)cutoffValue) + " Hz" : 
        juce::String(cutoffValue, 1) + " Hz";
    juce::String resonanceText = juce::String(resonanceValue, 1) + " Q";
    juce::String gainText = (gainValue >= 0.0f ? "+" : "") + juce::String(gainValue, 1) + " dB";
    
    drawRotaryKnob(nvgContext, knobSpacing * 0.5f, knobY, knobSize, 
                  cutoffSlider.getValue() / cutoffSlider.getMaximum(),
                  "Cutoff", cutoffText);
                  
    drawRotaryKnob(nvgContext, knobSpacing * 1.5f, knobY, knobSize,
                  resonanceSlider.getValue() / resonanceSlider.getMaximum(), 
                  "Resonance", resonanceText);
                  
    drawRotaryKnob(nvgContext, knobSpacing * 2.5f, knobY, knobSize,
                  (gainSlider.getValue() - gainSlider.getMinimum()) / 
                  (gainSlider.getMaximum() - gainSlider.getMinimum()),
                  "Gain", gainText);
    
    // Draw slope buttons
    float buttonY = 350.0f;
    drawButton(nvgContext, width * 0.5f - 130.0f, buttonY, 120.0f, 40.0f, 
              "6 dB/oct", slope6dBButton.getToggleState(), false);
    drawButton(nvgContext, width * 0.5f + 10.0f, buttonY, 120.0f, 40.0f,
              "18 dB/oct", slope18dBButton.getToggleState(), false);
    
    // Draw filter type buttons  
    buttonY = 450.0f;
    drawButton(nvgContext, width * 0.5f - 175.0f, buttonY, 110.0f, 40.0f,
              "Low-pass", lowPassButton.getToggleState(), false);
    drawButton(nvgContext, width * 0.5f - 55.0f, buttonY, 110.0f, 40.0f,
              "High-pass", highPassButton.getToggleState(), false);
    drawButton(nvgContext, width * 0.5f + 65.0f, buttonY, 110.0f, 40.0f,
              "Band-pass", bandPassButton.getToggleState(), false);
    
    // End NanoVG frame
    nvgEndFrame(nvgContext);
}

void NewPluginSkeletonAudioProcessorEditor::openGLContextClosing()
{
    if (nvgContext != nullptr)
    {
        nvgDeleteGL3(nvgContext);
        nvgContext = nullptr;
    }
}

void NewPluginSkeletonAudioProcessorEditor::timerCallback()
{
    animationTime += 1.0f / 60.0f; // Increment animation time
    repaint(); // Trigger OpenGL redraw
}

//==============================================================================
// NanoVG Drawing Methods

void NewPluginSkeletonAudioProcessorEditor::drawBackground(NVGcontext* vg, float width, float height)
{
    // Animated gradient background
    float t = sin(animationTime * 0.5f) * 0.5f + 0.5f;
    NVGcolor color1 = nvgRGBA(30 + t * 20, 58 + t * 30, 138 + t * 50, 255);
    NVGcolor color2 = nvgRGBA(49 + t * 20, 46 + t * 30, 129 + t * 40, 255);
    
    NVGpaint bg = nvgLinearGradient(vg, 0, 0, 0, height, color1, color2);
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, width, height);
    nvgFillPaint(vg, bg);
    nvgFill(vg);
}

void NewPluginSkeletonAudioProcessorEditor::drawRotaryKnob(NVGcontext* vg, float x, float y, float size, 
                                                          float value, const juce::String& label, 
                                                          const juce::String& valueText)
{
    float radius = size * 0.4f;
    float cx = x;
    float cy = y;
    
    // Draw outer shadow
    NVGpaint shadowPaint = nvgRadialGradient(vg, cx, cy + 2, radius * 0.8f, radius * 1.2f,
                                            nvgRGBA(0, 0, 0, 64), nvgRGBA(0, 0, 0, 0));
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy + 2, radius);
    nvgFillPaint(vg, shadowPaint);
    nvgFill(vg);
    
    // Draw knob base
    NVGpaint knobPaint = nvgRadialGradient(vg, cx, cy - radius * 0.3f, radius * 0.1f, radius,
                                          nvgRGBA(200, 200, 200, 255), nvgRGBA(100, 100, 100, 255));
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, radius);
    nvgFillPaint(vg, knobPaint);
    nvgFill(vg);
    
    // Draw knob rim
    nvgBeginPath(vg);
    nvgCircle(vg, cx, cy, radius);
    nvgStrokeColor(vg, nvgRGBA(50, 50, 50, 255));
    nvgStrokeWidth(vg, 2.0f);
    nvgStroke(vg);
    
    // Draw value arc
    float startAngle = -M_PI * 0.75f;
    float endAngle = M_PI * 0.75f;
    float currentAngle = startAngle + value * (endAngle - startAngle);
    
    nvgBeginPath(vg);
    nvgArc(vg, cx, cy, radius * 0.8f, startAngle, currentAngle, NVG_CW);
    nvgStrokeColor(vg, nvgRGBA(74, 144, 226, 255));
    nvgStrokeWidth(vg, 4.0f);
    nvgStroke(vg);
    
    // Draw pointer
    float pointerX = cx + sin(currentAngle) * radius * 0.7f;
    float pointerY = cy - cos(currentAngle) * radius * 0.7f;
    
    nvgBeginPath(vg);
    nvgCircle(vg, pointerX, pointerY, 4.0f);
    nvgFillColor(vg, nvgRGBA(74, 144, 226, 255));
    nvgFill(vg);
    
    // Draw label
    nvgFontFace(vg, "roboto-regular");
    nvgFontSize(vg, 14.0f);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(vg, cx, cy + radius + 20.0f, label.toRawUTF8(), nullptr);
    
    // Draw value
    nvgFontSize(vg, 12.0f);
    nvgFillColor(vg, nvgRGBA(200, 200, 200, 255));
    nvgText(vg, cx, cy + radius + 35.0f, valueText.toRawUTF8(), nullptr);
}

void NewPluginSkeletonAudioProcessorEditor::drawButton(NVGcontext* vg, float x, float y, float width, float height,
                                                       const juce::String& text, bool isSelected, bool isPressed)
{
    // Draw button background
    NVGcolor bgColor = isSelected ? nvgRGBA(74, 144, 226, 255) : nvgRGBA(255, 255, 255, 255);
    NVGcolor borderColor = nvgRGBA(50, 50, 50, 255);
    
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, width, height, 4.0f);
    nvgFillColor(vg, bgColor);
    nvgFill(vg);
    
    // Draw border
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, width, height, 4.0f);
    nvgStrokeColor(vg, borderColor);
    nvgStrokeWidth(vg, 2.0f);
    nvgStroke(vg);
    
    // Draw text
    NVGcolor textColor = isSelected ? nvgRGBA(255, 255, 255, 255) : nvgRGBA(0, 0, 0, 255);
    nvgFontFace(vg, "roboto-regular");
    nvgFontSize(vg, 14.0f);
    nvgFillColor(vg, textColor);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(vg, x + width * 0.5f, y + height * 0.5f, text.toRawUTF8(), nullptr);
}

void NewPluginSkeletonAudioProcessorEditor::drawPresetSection(NVGcontext* vg, float width)
{
    // Draw preset section background
    nvgBeginPath(vg);
    nvgRect(vg, 20, 10, width - 40, 30);
    nvgFillColor(vg, nvgRGBA(255, 255, 255, 200));
    nvgFill(vg);
    
    nvgBeginPath(vg);
    nvgRect(vg, 20, 10, width - 40, 30);
    nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgStrokeWidth(vg, 1.0f);
    nvgStroke(vg);
    
    // Draw "Select Preset..." text
    nvgFontFace(vg, "roboto-regular");
    nvgFontSize(vg, 12.0f);
    nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgText(vg, 30, 25, "Select Preset...", nullptr);
    
    // Draw Save button
    drawButton(vg, width - 140, 15, 60, 20, "Save", false, false);
    
    // Draw Load button  
    drawButton(vg, width - 70, 15, 60, 20, "Load", false, false);
}

//==============================================================================
// Mouse Handling

void NewPluginSkeletonAudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
{
    mousePos = event.getPosition();
}

void NewPluginSkeletonAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    isDragging = true;
    mousePos = event.getPosition();
}

void NewPluginSkeletonAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
    isDragging = false;
}

void NewPluginSkeletonAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        mousePos = event.getPosition();
    }
}

//==============================================================================
// Preset Management (keeping existing implementation)

void NewPluginSkeletonAudioProcessorEditor::savePreset()
{
    auto* nameWindow = new juce::AlertWindow("Save Preset", "Enter preset name:", juce::AlertWindow::NoIcon);
    nameWindow->addTextEditor("name", "New Preset", "Preset Name:");
    nameWindow->addButton("Save", 1);
    nameWindow->addButton("Cancel", 0);
    
    nameWindow->enterModalState(true, juce::ModalCallbackFunction::create([this, nameWindow](int result)
    {
        if (result == 1)
        {
            auto presetName = nameWindow->getTextEditorContents("name");
            if (presetName.isEmpty()) presetName = "New Preset";
            
            if (!currentPresetDirectory.exists())
                currentPresetDirectory.createDirectory();
                
            auto presetFile = currentPresetDirectory.getChildFile(presetName + ".fkpreset");
            
            juce::XmlElement preset("FrankyFilterPreset");
            preset.setAttribute("version", "1.0");
            preset.setAttribute("name", presetName);
            
            auto& params = audioProcessor.parameters;
            preset.setAttribute("cutoff", params.getParameter("cutoff")->getValue());
            preset.setAttribute("resonance", params.getParameter("resonance")->getValue());
            preset.setAttribute("gain", params.getParameter("gain")->getValue());
            preset.setAttribute("slope", params.getParameter("slope")->getValue());
            preset.setAttribute("filterType", params.getParameter("filterType")->getValue());
            
            if (preset.writeTo(presetFile))
            {
                updatePresetMenu();
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                                      "Success",
                                                      "Preset '" + presetName + "' saved!");
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                      "Error",
                                                      "Failed to save preset file.");
            }
        }
        
        delete nameWindow;
    }));
}

void NewPluginSkeletonAudioProcessorEditor::loadPreset()
{
    juce::Array<juce::File> presetFiles;
    currentPresetDirectory.findChildFiles(presetFiles, juce::File::findFiles, false, "*.fkpreset");
    
    if (presetFiles.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                              "No Presets Found",
                                              "No preset files found.");
        return;
    }
    
    juce::StringArray presetNames;
    for (auto& file : presetFiles)
        presetNames.add(file.getFileNameWithoutExtension());
    
    auto* loadWindow = new juce::AlertWindow("Load Preset", "Select preset to load:", juce::AlertWindow::NoIcon);
    loadWindow->addComboBox("presets", presetNames, "Available Presets:");
    loadWindow->addButton("Load", 1);
    loadWindow->addButton("Cancel", 0);
    
    loadWindow->enterModalState(true, juce::ModalCallbackFunction::create([this, presetFiles, loadWindow](int result)
    {
        if (result == 1)
        {
            auto selectedIndex = loadWindow->getComboBoxComponent("presets")->getSelectedItemIndex();
            if (selectedIndex >= 0 && selectedIndex < presetFiles.size())
            {
                auto presetFile = presetFiles[selectedIndex];
                auto preset = juce::XmlDocument::parse(presetFile);
                
                if (preset != nullptr)
                {
                    auto& params = audioProcessor.parameters;
                    
                    if (preset->hasAttribute("cutoff"))
                        params.getParameter("cutoff")->setValueNotifyingHost((float)preset->getDoubleAttribute("cutoff"));
                    if (preset->hasAttribute("resonance"))
                        params.getParameter("resonance")->setValueNotifyingHost((float)preset->getDoubleAttribute("resonance"));
                    if (preset->hasAttribute("gain"))
                        params.getParameter("gain")->setValueNotifyingHost((float)preset->getDoubleAttribute("gain"));
                    if (preset->hasAttribute("slope"))
                        params.getParameter("slope")->setValueNotifyingHost((float)preset->getDoubleAttribute("slope"));
                    if (preset->hasAttribute("filterType"))
                        params.getParameter("filterType")->setValueNotifyingHost((float)preset->getDoubleAttribute("filterType"));
                    
                    auto slopeIndex = dynamic_cast<juce::AudioParameterChoice*>(params.getParameter("slope"))->getIndex();
                    slope6dBButton.setToggleState(slopeIndex == 0, juce::dontSendNotification);
                    slope18dBButton.setToggleState(slopeIndex == 1, juce::dontSendNotification);
                    
                    auto typeIndex = dynamic_cast<juce::AudioParameterChoice*>(params.getParameter("filterType"))->getIndex();
                    lowPassButton.setToggleState(typeIndex == 0, juce::dontSendNotification);
                    highPassButton.setToggleState(typeIndex == 1, juce::dontSendNotification);
                    bandPassButton.setToggleState(typeIndex == 2, juce::dontSendNotification);
                }
            }
        }
        
        delete loadWindow;
    }));
}

void NewPluginSkeletonAudioProcessorEditor::loadPresetFromDropdown()
{
    // Implementation for dropdown loading
}

void NewPluginSkeletonAudioProcessorEditor::updatePresetMenu()
{
    presetMenu.clear();
    presetMenu.addItem("Select Preset...", 1);
    
    if (currentPresetDirectory.exists())
    {
        juce::Array<juce::File> presetFiles;
        currentPresetDirectory.findChildFiles(presetFiles, juce::File::findFiles, false, "*.fkpreset");
        
        for (int i = 0; i < presetFiles.size(); ++i)
            presetMenu.addItem(presetFiles[i].getFileNameWithoutExtension(), i + 2);
    }
    
    presetMenu.setSelectedId(1);
}

juce::File NewPluginSkeletonAudioProcessorEditor::getCurrentPresetDirectory()
{
    return currentPresetDirectory;
}

void NewPluginSkeletonAudioProcessorEditor::setCurrentPresetDirectory(const juce::File& directory)
{
    currentPresetDirectory = directory;
}

//==============================================================================
// JUCE Drawing Methods (Professional Interface)

void NewPluginSkeletonAudioProcessorEditor::drawJuceKnob(juce::Graphics& g, int x, int y, int size, 
                                                        float value, const juce::String& label, 
                                                        const juce::String& valueText)
{
    float radius = size * 0.35f;
    float cx = x + size * 0.5f;
    float cy = y + radius + 20;
    
    // Knob shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillEllipse(cx - radius + 2, cy - radius + 2, radius * 2, radius * 2);
    
    // Knob body gradient
    juce::ColourGradient knobGradient(juce::Colour(180, 185, 195), cx, cy - radius * 0.3f,
                                     juce::Colour(80, 85, 95), cx, cy + radius * 0.7f, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
    
    // Knob border
    g.setColour(juce::Colour(60, 65, 75));
    g.drawEllipse(cx - radius, cy - radius, radius * 2, radius * 2, 1.5f);
    
    // Knob indicator
    float angle = (value * 270.0f - 135.0f) * (M_PI / 180.0f);
    float indicatorLength = radius * 0.7f;
    
    g.setColour(juce::Colour(255, 120, 60));
    juce::Line<float> indicator(cx, cy, 
                               cx + cos(angle) * indicatorLength, 
                               cy + sin(angle) * indicatorLength);
    g.drawLine(indicator, 3.0f);
    
    // Center dot
    g.setColour(juce::Colour(100, 105, 115));
    g.fillEllipse(cx - 3, cy - 3, 6, 6);
    
    // Label
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 14.0f, juce::Font::plain));
    g.drawText(label, x, cy + radius + 15, size, 20, juce::Justification::centred);
    
    // Value
    g.setColour(juce::Colour(200, 205, 215));
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::plain));
    g.drawText(valueText, x, cy + radius + 32, size, 15, juce::Justification::centred);
}

void NewPluginSkeletonAudioProcessorEditor::drawJuceButton(juce::Graphics& g, int x, int y, int width, int height,
                                                          const juce::String& text, bool isSelected)
{
    // Button background
    juce::Colour bgColor = isSelected ? juce::Colour(100, 150, 255) : juce::Colour(60, 65, 80);
    
    g.setColour(bgColor);
    g.fillRoundedRectangle(x, y, width, height, 4.0f);
    
    // Button border
    g.setColour(juce::Colour(120, 125, 140));
    g.drawRoundedRectangle(x, y, width, height, 4.0f, 1.0f);
    
    // Button text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 13.0f, juce::Font::plain));
    g.drawText(text, x, y, width, height, juce::Justification::centred);
}

juce::String NewPluginSkeletonAudioProcessorEditor::formatCutoffValue(float value)
{
    return value >= 1000.0f ? 
        juce::String((int)value) + " Hz" : 
        juce::String(value, 1) + " Hz";
}

juce::String NewPluginSkeletonAudioProcessorEditor::formatResonanceValue(float value)
{
    return juce::String(value, 1) + " Q";
}

juce::String NewPluginSkeletonAudioProcessorEditor::formatGainValue(float value)
{
    return (value >= 0.0f ? "+" : "") + juce::String(value, 1) + " dB";
}

//==============================================================================
// Listener Implementations

void NewPluginSkeletonAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    repaint(); // Redraw when slider values change
}

void NewPluginSkeletonAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    repaint(); // Redraw when button states change
}

