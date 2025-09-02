/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

//==============================================================================
/**
    MyAwesome Filter - Low-pass filter plugin with variable slope
*/
class NewPluginSkeletonAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewPluginSkeletonAudioProcessor();
    ~NewPluginSkeletonAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Plugin-specific methods
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Public access to parameters for editor
    juce::AudioProcessorValueTreeState parameters;
    
private:
    
    // Parameter pointers
    juce::AudioParameterFloat* cutoffFreq = nullptr;
    juce::AudioParameterFloat* resonance = nullptr;
    juce::AudioParameterChoice* filterSlope = nullptr;
    juce::AudioParameterChoice* filterType = nullptr;
    juce::AudioParameterFloat* gain = nullptr;
    
    // Parameter smoothing
    juce::SmoothedValue<float> cutoffSmoother, resonanceSmoother, gainSmoother;
    juce::SmoothedValue<float> slopeSmoother; // For click-free slope transitions
    
    // DSP processing components - Cascaded filters for different slopes
    // 6dB/oct: 1 filter
    // 12dB/oct: 2 filters cascaded
    // 24dB/oct: 4 filters cascaded
    static constexpr int maxFilterStages = 4;
    std::array<juce::dsp::StateVariableTPTFilter<float>, maxFilterStages> filterChain;
    
    juce::dsp::Limiter<float> outputLimiter; // Prevent signal exceeding -0.1dB
    
    // Filter state variables for multichannel processing
    int numChannels = 2;
    
    // Helper function to get number of filter stages for slope
    int getSlopeFilterStages(int slopeIndex) const;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewPluginSkeletonAudioProcessor)
};
