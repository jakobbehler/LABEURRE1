/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Extract Parameters


// A structure containing all the parameters of the plugin
struct ChainSettings
{
    float bandsplit_frequency {0},  compLowIntensity {0}, compHighIntensity {0}, distLowIntensity {0}, distHighIntensity {0};
    int compressorSpeed {0}, distortionType {0} ;
    //float lowCutFreq{0}, highCutFreq{0};
    
    //Slope lowCutSlope{Slope::Slope_12},  highCutSlope{Slope::Slope_12};
};


struct CompressorSettings {
    float threshold;
    float ratio;
    float makeupGain;
};

struct distortionSettings {
    float drive;
    float c;
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);



//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
    public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;
    
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
    
    // Add parameters ----------------------
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};
    
    
    
    
    
    
    
    private:
    
    // Type Aliases and Processor Chains

    // Define filters and compressors
    using Filter = juce::dsp::IIR::Filter<float>;
    using Compressor = juce::dsp::Compressor<float>;
    using Gain = juce::dsp::Gain<float>;

    // Crossover filter (Linkwitz-Riley)
    using Crossover = juce::dsp::LinkwitzRileyFilter<float>;

    juce::dsp::IIR::Filter<float> lowPassFilter;
    // Low-band chain: Low-pass + Compressor
    using LowBandChain = juce::dsp::ProcessorChain<Filter, Compressor, Gain>;

    // High-band chain: High-pass + Compressor
    using HighBandChain = juce::dsp::ProcessorChain<Filter, Compressor, Gain>;

    // Full Processor Chain: Crossover, then two independent bands
    using MultiBandCompressorChain = juce::dsp::ProcessorChain<
        Crossover, // Band-splitter
        LowBandChain,
        HighBandChain
        
    >;

   
    MultiBandCompressorChain leftChain, rightChain;
    
  
    //void updatePeakFilter(const ChainSettings& chainSettings);
    
    //datatype of the IIR filter coeffs
    using Coefficients = Filter::CoefficientsPtr;
    
    // parameters update funtions
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    
    // COMPRESSOR METHODS -----------------------------
    
    CompressorSettings getCompressorSettings(const double intensity);
    void applyCompressorSettings(juce::dsp::Compressor<float>& compressor, juce::dsp::Gain<float>& gain, const CompressorSettings& settings, int compressorSpeed);
    void updateCompressor();
    
    // DISTORTION METHODS -----------------------------
    
    float distortionSample(float x, float y_old, float drive, float c);
    
    float distortionWarm(float x, float y_old, float drive, float c);
    float distortionCrush(float x, float y_old, float drive, float c);
    
    float asymmetricSoftClip(float x, float posThreshold = 1.0f, float negThreshold = -0.8f);
    
    distortionSettings getDistortionSettings(const double intensity);
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
