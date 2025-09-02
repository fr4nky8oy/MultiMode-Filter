#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "../Source/PluginProcessor.h"
#include <cmath>

class SlopeSwitchingTest : public juce::UnitTest
{
public:
    SlopeSwitchingTest() : juce::UnitTest("Slope Switching Test") {}
    
    void runTest() override
    {
        beginTest("Smooth slope switching without artifacts");
        
        // Create processor
        NewPluginSkeletonAudioProcessor processor;
        
        // Prepare processor
        double sampleRate = 48000.0;
        int bufferSize = 512;
        processor.setPlayConfigDetails(2, 2, sampleRate, bufferSize);
        processor.prepareToPlay(sampleRate, bufferSize);
        
        // Set cutoff to 1kHz
        auto* cutoffParam = processor.parameters.getParameter("cutoff");
        if (cutoffParam)
        {
            cutoffParam->setValueNotifyingHost(processor.parameters.getParameterRange("cutoff")
                .convertTo0to1(1000.0f));
        }
        
        // Set resonance to neutral
        auto* resParam = processor.parameters.getParameter("resonance");
        if (resParam)
        {
            resParam->setValueNotifyingHost(processor.parameters.getParameterRange("resonance")
                .convertTo0to1(0.707f));
        }
        
        // Generate continuous test signal
        juce::AudioBuffer<float> testBuffer(2, bufferSize * 3);
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < testBuffer.getNumSamples(); ++i)
            {
                // Mix of frequencies to test
                float phase1 = (2.0f * juce::MathConstants<float>::pi * 440.0f * i) / sampleRate;
                float phase2 = (2.0f * juce::MathConstants<float>::pi * 880.0f * i) / sampleRate;
                float phase3 = (2.0f * juce::MathConstants<float>::pi * 1760.0f * i) / sampleRate;
                float sample = (std::sin(phase1) + 0.5f * std::sin(phase2) + 0.25f * std::sin(phase3)) / 1.75f;
                testBuffer.setSample(ch, i, sample);
            }
        }
        
        // Process first block with 6dB slope
        auto* slopeParam = processor.parameters.getParameter("slope");
        slopeParam->setValueNotifyingHost(0.0f); // 6dB
        
        juce::AudioBuffer<float> block1(testBuffer.getArrayOfWritePointers(), 2, 0, bufferSize);
        juce::MidiBuffer midiBuffer;
        processor.processBlock(block1, midiBuffer);
        
        // Switch to 12dB slope for second block
        slopeParam->setValueNotifyingHost(0.5f); // 12dB
        
        juce::AudioBuffer<float> block2(testBuffer.getArrayOfWritePointers(), 2, bufferSize, bufferSize);
        processor.processBlock(block2, midiBuffer);
        
        // Switch to 24dB slope for third block
        slopeParam->setValueNotifyingHost(1.0f); // 24dB
        
        juce::AudioBuffer<float> block3(testBuffer.getArrayOfWritePointers(), 2, bufferSize * 2, bufferSize);
        processor.processBlock(block3, midiBuffer);
        
        // Check for discontinuities at block boundaries
        checkContinuity(testBuffer, bufferSize - 1, bufferSize, "6dB to 12dB transition");
        checkContinuity(testBuffer, bufferSize * 2 - 1, bufferSize * 2, "12dB to 24dB transition");
        
        // Check for no NaN or Inf values
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < testBuffer.getNumSamples(); ++i)
            {
                float sample = testBuffer.getSample(ch, i);
                expect(!std::isnan(sample) && !std::isinf(sample),
                       "Sample at index " + juce::String(i) + " is NaN or Inf");
            }
        }
    }
    
private:
    void checkContinuity(const juce::AudioBuffer<float>& buffer, int idx1, int idx2, const juce::String& description)
    {
        float sample1 = buffer.getSample(0, idx1);
        float sample2 = buffer.getSample(0, idx2);
        float discontinuity = std::abs(sample2 - sample1);
        
        // Allow for some smoothing but flag large jumps
        expect(discontinuity < 0.1f,
               description + ": Discontinuity of " + juce::String(discontinuity) + " detected");
    }
};

static SlopeSwitchingTest slopeSwitchingTest;