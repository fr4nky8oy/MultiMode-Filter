/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewPluginSkeletonAudioProcessor::NewPluginSkeletonAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    // Get parameter pointers
    cutoffFreq = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("cutoff"));
    resonance = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("resonance"));
    filterSlope = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("slope"));
    filterType = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("filterType"));
    gain = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("gain"));
}

NewPluginSkeletonAudioProcessor::~NewPluginSkeletonAudioProcessor()
{
}

//==============================================================================
const juce::String NewPluginSkeletonAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewPluginSkeletonAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewPluginSkeletonAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewPluginSkeletonAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewPluginSkeletonAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewPluginSkeletonAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewPluginSkeletonAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewPluginSkeletonAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewPluginSkeletonAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewPluginSkeletonAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewPluginSkeletonAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize DSP components
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    numChannels = getTotalNumOutputChannels();
    
    // Prepare all filter stages in the chain
    for (auto& filter : filterChain)
    {
        filter.prepare(spec);
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    }
    
    // Prepare output limiter to prevent exceeding -0.1dB
    outputLimiter.prepare(spec);
    outputLimiter.setThreshold(-0.1f); // -0.1dB threshold
    outputLimiter.setRelease(5.0f);    // Fast 5ms release time
    
    // Initialize parameter smoothers
    cutoffSmoother.reset(sampleRate, 0.05); // 50ms ramp
    resonanceSmoother.reset(sampleRate, 0.02); // 20ms ramp
    gainSmoother.reset(sampleRate, 0.02); // 20ms ramp
    slopeSmoother.reset(sampleRate, 0.1); // 100ms ramp for smooth slope transitions
    
    // Set initial parameter values
    if (cutoffFreq != nullptr && resonance != nullptr && filterSlope != nullptr && gain != nullptr)
    {
        cutoffSmoother.setCurrentAndTargetValue(cutoffFreq->get());
        resonanceSmoother.setCurrentAndTargetValue(resonance->get());
        gainSmoother.setCurrentAndTargetValue(gain->get());
        slopeSmoother.setCurrentAndTargetValue(static_cast<float>(filterSlope->getIndex()));
    }
}

void NewPluginSkeletonAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewPluginSkeletonAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NewPluginSkeletonAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update parameter smoothers
    if (cutoffFreq != nullptr)
        cutoffSmoother.setTargetValue(cutoffFreq->get());
    if (resonance != nullptr)
        resonanceSmoother.setTargetValue(resonance->get());
    if (gain != nullptr)
        gainSmoother.setTargetValue(gain->get());
    if (filterSlope != nullptr)
        slopeSmoother.setTargetValue(static_cast<float>(filterSlope->getIndex()));

    // Get current parameter settings
    int slope = filterSlope != nullptr ? filterSlope->getIndex() : 0; // Default 6dB
    int filterTypeIndex = filterType != nullptr ? filterType->getIndex() : 0; // Default Low-pass
    
    // Map filter type index to StateVariableTPTFilterType
    juce::dsp::StateVariableTPTFilterType filterMode;
    switch (filterTypeIndex)
    {
        case 0: filterMode = juce::dsp::StateVariableTPTFilterType::lowpass; break;   // Low-pass
        case 1: filterMode = juce::dsp::StateVariableTPTFilterType::highpass; break;  // High-pass
        case 2: filterMode = juce::dsp::StateVariableTPTFilterType::bandpass; break;  // Band-pass
        default: filterMode = juce::dsp::StateVariableTPTFilterType::lowpass; break;
    }
    
    auto numSamples = buffer.getNumSamples();
    
    // Update filter parameters for each sample (sample-accurate automation)
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float currentCutoff = cutoffSmoother.getNextValue();
        float currentResonance = resonanceSmoother.getNextValue();
        float currentGainDB = gainSmoother.getNextValue();
        float currentSlopeSmooth = slopeSmoother.getNextValue();
        
        // Convert gain from dB to linear
        float gainLinear = juce::Decibels::decibelsToGain(currentGainDB);
        
        // Get integer slope indices for crossfading
        int currentSlopeIndex = static_cast<int>(std::round(currentSlopeSmooth));
        int prevSlopeIndex = currentSlopeIndex;
        float crossfadeAmount = 0.0f;
        
        // Calculate crossfade if we're between slopes
        if (currentSlopeSmooth != static_cast<float>(currentSlopeIndex))
        {
            prevSlopeIndex = static_cast<int>(std::floor(currentSlopeSmooth));
            int nextSlopeIndex = static_cast<int>(std::ceil(currentSlopeSmooth));
            if (prevSlopeIndex != nextSlopeIndex)
            {
                crossfadeAmount = currentSlopeSmooth - static_cast<float>(prevSlopeIndex);
                currentSlopeIndex = nextSlopeIndex;
            }
        }
        
        // Update all filters with current parameters
        // For proper Butterworth response, distribute Q across stages
        int numStages = getSlopeFilterStages(currentSlopeIndex);
        float stageResonance = currentResonance;
        
        // For cascaded filters, adjust Q per stage for proper Butterworth response
        if (numStages > 1)
        {
            // Butterworth Q values: 2-pole = 0.707, 4-pole cascaded = 0.54 and 1.31
            if (numStages == 2)
                stageResonance = currentResonance * 0.707f / 0.707f; // Normalized
            else if (numStages == 4)
                stageResonance = currentResonance * 0.54f / 0.707f; // Use lower Q for stability
        }
        
        for (int stage = 0; stage < maxFilterStages; ++stage)
        {
            filterChain[stage].setCutoffFrequency(currentCutoff);
            filterChain[stage].setResonance(stageResonance);
            filterChain[stage].setType(filterMode);
        }
        
        // Process each channel through the cascaded filter chain
        for (int ch = 0; ch < totalNumInputChannels; ++ch)
        {
            float inputSample = buffer.getSample(ch, sample);
            float outputSample = inputSample;
            
            // Process through active filter stages
            int activeStages = getSlopeFilterStages(currentSlopeIndex);
            for (int stage = 0; stage < activeStages; ++stage)
            {
                outputSample = filterChain[stage].processSample(ch, outputSample);
            }
            
            // If crossfading, also process through previous slope configuration
            if (crossfadeAmount > 0.0f && prevSlopeIndex != currentSlopeIndex)
            {
                float prevOutputSample = inputSample;
                int prevStages = getSlopeFilterStages(prevSlopeIndex);
                for (int stage = 0; stage < prevStages; ++stage)
                {
                    prevOutputSample = filterChain[stage].processSample(ch, prevOutputSample);
                }
                // Crossfade between the two slope outputs
                outputSample = prevOutputSample * (1.0f - crossfadeAmount) + outputSample * crossfadeAmount;
            }
            
            // Apply post-filter gain
            outputSample *= gainLinear;
            
            buffer.setSample(ch, sample, outputSample);
        }
    }
    
    // Apply output limiting to ensure signal never exceeds -0.1dB
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    outputLimiter.process(context);
}

//==============================================================================
bool NewPluginSkeletonAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewPluginSkeletonAudioProcessor::createEditor()
{
    return new NewPluginSkeletonAudioProcessorEditor (*this);
}

//==============================================================================
void NewPluginSkeletonAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NewPluginSkeletonAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout NewPluginSkeletonAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Cutoff frequency parameter (20Hz - 20kHz, logarithmic)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "Cutoff Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f,
        "Hz"));
    
    // Resonance parameter (0.1 - 5.0, linear)  
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Resonance",
        juce::NormalisableRange<float>(0.1f, 5.0f, 0.01f), 0.707f,
        "Q"));
    
    // Filter slope parameter (6dB, 12dB, 24dB)
    juce::StringArray slopeChoices = {"6 dB/oct", "12 dB/oct", "24 dB/oct"};
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "slope", "Filter Slope", slopeChoices, 0)); // Default to 6dB
    
    // Filter type parameter (Low-pass, High-pass, Band-pass)
    juce::StringArray typeChoices = {"Low-pass", "High-pass", "Band-pass"};
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        "filterType", "Filter Type", typeChoices, 0)); // Default to Low-pass
    
    // Post-filter gain parameter (-24dB to +12dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f,
        "dB"));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewPluginSkeletonAudioProcessor();
}

//==============================================================================
int NewPluginSkeletonAudioProcessor::getSlopeFilterStages(int slopeIndex) const
{
    switch (slopeIndex)
    {
        case 0: return 1; // 6dB/oct - 1 stage (1-pole equivalent)
        case 1: return 2; // 12dB/oct - 2 stages (2-pole)
        case 2: return 4; // 24dB/oct - 4 stages (4-pole)
        default: return 1;
    }
}
