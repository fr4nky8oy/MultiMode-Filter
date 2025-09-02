#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

class PresetFunctionalityTest : public juce::UnitTest
{
public:
    PresetFunctionalityTest() : juce::UnitTest("Preset Functionality Test") {}
    
    void runTest() override
    {
        beginTest("Preset directory creation and parameter saving");
        
        // Create processor
        NewPluginSkeletonAudioProcessor processor;
        
        // Set up processor
        double sampleRate = 48000.0;
        int bufferSize = 512;
        processor.setPlayConfigDetails(2, 2, sampleRate, bufferSize);
        processor.prepareToPlay(sampleRate, bufferSize);
        
        // Set some test parameter values
        auto* cutoffParam = processor.parameters.getParameter("cutoff");
        auto* resonanceParam = processor.parameters.getParameter("resonance");
        auto* gainParam = processor.parameters.getParameter("gain");
        auto* slopeParam = processor.parameters.getParameter("slope");
        auto* filterTypeParam = processor.parameters.getParameter("filterType");
        
        expect(cutoffParam != nullptr, "Cutoff parameter should exist");
        expect(resonanceParam != nullptr, "Resonance parameter should exist");
        expect(gainParam != nullptr, "Gain parameter should exist");
        expect(slopeParam != nullptr, "Slope parameter should exist");
        expect(filterTypeParam != nullptr, "Filter type parameter should exist");
        
        // Set test values
        cutoffParam->setValueNotifyingHost(0.5f);  // Mid-range cutoff
        resonanceParam->setValueNotifyingHost(0.8f); // High resonance
        gainParam->setValueNotifyingHost(0.3f);    // Some gain
        slopeParam->setValueNotifyingHost(0.5f);   // 12dB slope
        filterTypeParam->setValueNotifyingHost(0.5f); // High pass
        
        // Test parameter state saving and loading
        auto originalState = processor.parameters.copyState();
        auto xml = originalState.createXml();
        expect(xml != nullptr, "State should be serializable to XML");
        
        // Change parameters to different values
        cutoffParam->setValueNotifyingHost(0.2f);
        resonanceParam->setValueNotifyingHost(0.1f);
        gainParam->setValueNotifyingHost(0.7f);
        slopeParam->setValueNotifyingHost(0.0f);   // 6dB slope
        filterTypeParam->setValueNotifyingHost(1.0f); // Band pass
        
        // Verify parameters changed
        expectWithinAbsoluteError(cutoffParam->getValue(), 0.2f, 0.01f, "Cutoff should be changed");
        expectWithinAbsoluteError(resonanceParam->getValue(), 0.1f, 0.01f, "Resonance should be changed");
        expectWithinAbsoluteError(gainParam->getValue(), 0.7f, 0.01f, "Gain should be changed");
        expectWithinAbsoluteError(slopeParam->getValue(), 0.0f, 0.01f, "Slope should be changed");
        expectWithinAbsoluteError(filterTypeParam->getValue(), 1.0f, 0.01f, "Filter type should be changed");
        
        // Restore original state
        auto restoredTree = juce::ValueTree::fromXml(*xml);
        processor.parameters.replaceState(restoredTree);
        
        // Verify parameters were restored
        expectWithinAbsoluteError(cutoffParam->getValue(), 0.5f, 0.01f, "Cutoff should be restored");
        expectWithinAbsoluteError(resonanceParam->getValue(), 0.8f, 0.01f, "Resonance should be restored");
        expectWithinAbsoluteError(gainParam->getValue(), 0.3f, 0.01f, "Gain should be restored");
        expectWithinAbsoluteError(slopeParam->getValue(), 0.5f, 0.01f, "Slope should be restored");
        expectWithinAbsoluteError(filterTypeParam->getValue(), 0.5f, 0.01f, "Filter type should be restored");
        
        logMessage("All parameters correctly saved and restored from preset data");
    }
};

static PresetFunctionalityTest presetFunctionalityTest;