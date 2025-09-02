#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "../Source/PluginProcessor.h"
#include <cmath>

class TwelveDbFilterTest : public juce::UnitTest
{
public:
    TwelveDbFilterTest() : juce::UnitTest("12dB Filter Test") {}
    
    void runTest() override
    {
        beginTest("12dB/octave slope verification");
        
        // Create processor
        NewPluginSkeletonAudioProcessor processor;
        
        // Prepare processor
        double sampleRate = 48000.0;
        int bufferSize = 512;
        processor.setPlayConfigDetails(2, 2, sampleRate, bufferSize);
        processor.prepareToPlay(sampleRate, bufferSize);
        
        // Set filter to 12dB/oct slope
        auto* slopeParam = processor.parameters.getParameter("slope");
        if (slopeParam)
        {
            slopeParam->setValueNotifyingHost(1.0f / 2.0f); // Index 1 = 12dB
        }
        
        // Set cutoff to 1kHz
        auto* cutoffParam = processor.parameters.getParameter("cutoff");
        if (cutoffParam)
        {
            cutoffParam->setValueNotifyingHost(processor.parameters.getParameterRange("cutoff")
                .convertTo0to1(1000.0f));
        }
        
        // Set resonance to neutral (0.707)
        auto* resParam = processor.parameters.getParameter("resonance");
        if (resParam)
        {
            resParam->setValueNotifyingHost(processor.parameters.getParameterRange("resonance")
                .convertTo0to1(0.707f));
        }
        
        // Process some blocks to stabilize
        juce::AudioBuffer<float> warmupBuffer(2, bufferSize);
        juce::MidiBuffer midiBuffer;
        for (int i = 0; i < 10; ++i)
        {
            processor.processBlock(warmupBuffer, midiBuffer);
        }
        
        // Test frequency response at different frequencies
        testFrequencyResponse(processor, 500.0f, sampleRate, bufferSize, 0.0f, 2.0f);   // Below cutoff
        testFrequencyResponse(processor, 1000.0f, sampleRate, bufferSize, -3.0f, 2.0f);  // At cutoff (-3dB)
        testFrequencyResponse(processor, 2000.0f, sampleRate, bufferSize, -12.0f, 3.0f); // 1 octave above (-12dB)
        testFrequencyResponse(processor, 4000.0f, sampleRate, bufferSize, -24.0f, 4.0f); // 2 octaves above (-24dB)
    }
    
private:
    void testFrequencyResponse(NewPluginSkeletonAudioProcessor& processor,
                               float testFreq, double sampleRate, int bufferSize,
                               float expectedDb, float tolerance)
    {
        // Generate test sine wave
        juce::AudioBuffer<float> testBuffer(2, bufferSize);
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < bufferSize; ++i)
            {
                float phase = (2.0f * juce::MathConstants<float>::pi * testFreq * i) / sampleRate;
                testBuffer.setSample(ch, i, std::sin(phase));
            }
        }
        
        // Measure input RMS
        float inputRms = 0.0f;
        for (int i = 0; i < bufferSize; ++i)
        {
            float sample = testBuffer.getSample(0, i);
            inputRms += sample * sample;
        }
        inputRms = std::sqrt(inputRms / bufferSize);
        
        // Process through filter
        juce::MidiBuffer midiBuffer;
        processor.processBlock(testBuffer, midiBuffer);
        
        // Measure output RMS
        float outputRms = 0.0f;
        for (int i = bufferSize / 2; i < bufferSize; ++i) // Skip first half to avoid transients
        {
            float sample = testBuffer.getSample(0, i);
            outputRms += sample * sample;
        }
        outputRms = std::sqrt(outputRms / (bufferSize / 2));
        
        // Calculate gain in dB
        float gainDb = 20.0f * std::log10(outputRms / (inputRms + 1e-10f));
        
        // Check if within tolerance
        expect(std::abs(gainDb - expectedDb) < tolerance,
               juce::String("Frequency ") + juce::String(testFreq) + 
               "Hz: Expected " + juce::String(expectedDb) + 
               "dB, got " + juce::String(gainDb) + "dB");
    }
};

static TwelveDbFilterTest twelveDbFilterTest;