/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "frequencyLines.h"
// Extract Parameters


// A structure containing all the parameters of the plugin
struct ChainSettings
{
    float bandsplit_frequency {0},  compLowIntensity {0}, compHighIntensity {0}, distLowIntensity {0}, distHighIntensity {0}, highCutFreq {0};
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

struct UpwardCompressorSettings
{
    float threshold;
    float ratio;
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);





class FFTDataGenerator
{
public:
    
    void setSampleRate(float newSampleRate) { sampleRate = newSampleRate; }
    
    static constexpr int fftOrder = 11; // 2^11 = 2048 samples
    static constexpr int fftSize = 1 << fftOrder;

    FFTDataGenerator() : forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        juce::zeromem(fftData, sizeof(fftData));
    }

    // Feed in audio data here
    void pushSamples(const juce::AudioBuffer<float>& buffer);

    // Do FFT and return magnitude bins in dB
    bool produceFFTData(std::vector<float>& outputBins);

private:
    float sampleRate = 44100.0f; // default fallback
    
    float fifo[fftSize] = { 0 };
    float fftData[fftSize * 2] = { 0 };
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
};




//===================================================================================================================
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
    
    
    FFTDataGenerator fftData;
    std::vector<float> fftBins;
    const std::vector<float>& getFftData() const { return fftBins; } // Getter for the editor

    
    
    
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
        HighBandChain,
        Filter
    >;

   
    MultiBandCompressorChain leftChain, rightChain;
    
  
    //void updatePeakFilter(const ChainSettings& chainSettings);
    
    //datatype of the IIR filter coeffs
    using Coefficients = Filter::CoefficientsPtr;
    
    // parameters update funtions
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    
    // COMPRESSOR METHODS -----------------------------
    
    CompressorSettings getCompressorSettings(const double intensity);
    int getCompressorSpeedMode();
    UpwardCompressorSettings getUpwardCompSettings(const double intensity);
    
    void applyCompressorSettings(juce::dsp::Compressor<float>& compressor, juce::dsp::Gain<float>& gain, const CompressorSettings& settings, int compressorSpeed);
    
    
    void updateCompressor();
    
    void applyUpwardCompression(float& lowSample, float& highSample, float compLowIntensity, float compHighIntensity);
    std::pair<float, float> applyUpwardCompression(float low, float high);
    
    void updateFilter();
    // DISTORTION METHODS -----------------------------
    
    float distortionSample(float x, float y_old, float drive, float c);
    
    float distortionWarm(float x, float y_old, float drive, float c);
    float distortionCrush(float x, float y_old, float drive, float c);
    float distortionDONT(float x, float y_old, float drive, float c);
   
    float asymmetricSoftClip(float x, float posThreshold = 1.0f, float negThreshold = -0.8f);
    
    distortionSettings getDistortionSettings(const double intensity);
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};





//
//class AudioBufferQueue
//{
//public:
//    static constexpr int bufferSize = 512;
//    static constexpr int numBuffers = 20;
//
//    void push(const juce::AudioBuffer<float>& buffer);
//
//    const juce::AudioBuffer<float>& getLatest();
//
//private:
//    juce::AbstractFifo fifo { numBuffers };
//    std::array<juce::AudioBuffer<float>, numBuffers> buffers;
//    juce::AudioBuffer<float> dummy;
//};
//AudioBufferQueue bufferQueue;
//

