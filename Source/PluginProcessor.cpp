/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
// PREPARE TO PLAY
//==============================================================================

void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize left and right channel and their DSP chains
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    
    leftChain.prepare(spec);
    rightChain.prepare(spec);
    
   
    
    float crossoverFreq = apvts.getRawParameterValue("bandsplit_frequency")->load();

    // Set crossover filter cutoff
    leftChain.get<0>().setCutoffFrequency(crossoverFreq);
    rightChain.get<0>().setCutoffFrequency(crossoverFreq);
    
    updateCompressor();
    
    //*lowPassFilter.state = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 5000.0f);
    
}



void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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


//==============================================================================
// PROCESS BLOCK
//==============================================================================

void SimpleEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;


    updateCompressor();
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Get distortion parameters for both bands
    float distHigh = apvts.getRawParameterValue("distHighIntensity")->load();
    float distLow = apvts.getRawParameterValue("distLowIntensity")->load();

    distortionSettings highSettings = getDistortionSettings(distHigh);
    distortionSettings lowSettings = getDistortionSettings(distLow);

    // Ensure the extra output channels are cleared
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get real-time crossover frequency
    float crossoverFreq = apvts.getRawParameterValue("bandsplit_frequency")->load();
    leftChain.get<0>().setCutoffFrequency(crossoverFreq);
    rightChain.get<0>().setCutoffFrequency(crossoverFreq);

    // Process each channel independently
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto& chain = (channel == 0) ? leftChain : rightChain;

        // Store previous samples for hysteresis feedback
        float y_old_low = 0.f;
        float y_old_high = 0.f;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float inputSample = channelData[sample];

            // Split signal into low and high frequency bands
            float lowSample, highSample;
            chain.get<0>().processSample(channel, inputSample, lowSample, highSample);

            // 🎸 Apply tape distortion
            lowSample = distortionSample(lowSample, y_old_low, lowSettings.drive, lowSettings.c);
            highSample = distortionSample(highSample, y_old_high, highSettings.drive, highSettings.c);

            // Store previous output for hysteresis feedback
            y_old_low = lowSample;
            y_old_high = highSample;

            // 🎛 Apply compression to each band
            lowSample = chain.get<1>().get<1>().processSample(channel, lowSample);
            highSample = chain.get<2>().get<1>().processSample(channel, highSample);

            // 🔊 Apply Gain (Makeup Compensation)
            lowSample = chain.get<1>().get<2>().processSample(lowSample);
            highSample = chain.get<2>().get<2>().processSample(highSample);

            // 🎚 Sum processed bands
            channelData[sample] = lowSample + highSample;
        }
    }
}



//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    //return new SimpleEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
    
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
     
        updateCompressor();
    }
}

//=========================================================================================================

// PARAMETER INITIALIZATION

//=========================================================================================================


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.bandsplit_frequency = apvts.getRawParameterValue("bandsplit_frequency") ->load();
    
    settings.compHighIntensity = apvts.getRawParameterValue("compHighIntensity") ->load();
    settings.compLowIntensity = apvts.getRawParameterValue("compLowIntensity") ->load();
    
    settings.distLowIntensity = apvts.getRawParameterValue("distLowIntensity") ->load();
    settings.distHighIntensity = apvts.getRawParameterValue("distHighIntensity") ->load();
    
    settings.compressorSpeed = static_cast<int>(apvts.getRawParameterValue("compressorSpeed")->load());
    settings.distortionType = static_cast<int>(apvts.getRawParameterValue("distortionType")->load());

    
    return settings;
}


// COEFFICIENT SETTING HELPERS -----------------------------------------------

// COMPRESSOR ----------

CompressorSettings SimpleEQAudioProcessor::getCompressorSettings(const double intensity){
    
    CompressorSettings settings;
    
    
    settings.threshold = intensity * (-30.f);  //
    settings.ratio = 1.f + intensity * 35.0f;
    
    //settings.makeupGain = 1.f - (settings.threshold / settings.ratio) * (1 - (1.0f / settings.ratio));
    
    //settings.makeupGain = 1.f - 4.0*(settings.threshold / settings.ratio) * (1 - (6.0f / settings.ratio));
    settings.makeupGain = 1.f + 5.f*(intensity+1.f)*(intensity+1.f);
    
    return settings;
    
}


void SimpleEQAudioProcessor::applyCompressorSettings(juce::dsp::Compressor<float>& compressor, juce::dsp::Gain<float>& gain, const CompressorSettings& settings, int compressorSpeed)
{
    compressor.setThreshold(settings.threshold);
    compressor.setRatio(settings.ratio);
    
    gain.setGainDecibels(settings.makeupGain);
    
    float fastestAttack = 1000.0f / getSampleRate();
    // Attack & Release settings based on compressor speed (0 = Fast, 1 = Slow)
    const std::array<float, 2> attackTimes = {fastestAttack, 100.0f};  // ms
    const std::array<float, 2> releaseTimes = {50.0f, 200.0f}; // ms

    compressor.setAttack(attackTimes[compressorSpeed]);
    compressor.setRelease(releaseTimes[compressorSpeed]);
    
}


void SimpleEQAudioProcessor::updateCompressor()
{
    
    const ChainSettings chainSettings = getChainSettings(apvts);
    // Define compressor settings for both bands
    CompressorSettings lowBandSettings = getCompressorSettings(chainSettings.compLowIntensity);
    CompressorSettings highBandSettings = getCompressorSettings(chainSettings.compHighIntensity);
    
    const int compSpeed = chainSettings.compressorSpeed;

    // Apply settings to both stereo channels (Compressor + Gain)
    applyCompressorSettings(leftChain.get<1>().get<1>(), leftChain.get<1>().get<2>(), lowBandSettings, compSpeed);  // Low-band
    applyCompressorSettings(leftChain.get<2>().get<1>(), leftChain.get<2>().get<2>(), highBandSettings, compSpeed); // High-band

    applyCompressorSettings(rightChain.get<1>().get<1>(), rightChain.get<1>().get<2>(), lowBandSettings, compSpeed); // Low-band
    applyCompressorSettings(rightChain.get<2>().get<1>(), rightChain.get<2>().get<2>(), highBandSettings, compSpeed); // High-band
}





void SimpleEQAudioProcessor::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}



// CREATING PARAMETERS ---------------------------------------------------------


juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("bandsplit_frequency", 1),
                                                                 "bandsplit_frequency",
                                                                 juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                                 750.f));
    // COMP ----
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("compLowIntensity", 1),
                                                                 "compLowIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 0.75f),
                                                                 0.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("compHighIntensity", 1),
                                                                 "compHighIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 0.75f),
                                                                 0.f));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("compressorSpeed", 1),
                                                            "Compressor Speed",
                                                            juce::StringArray{"Fast", "Slow"}, // Choices
                                                            0 ));
    
                                               
    
    // DISTORTION ----
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("distLowIntensity", 1),
                                                                 "distLowIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f),
                                                                 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("distHighIntensity", 1),
                                                                 "distHighIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f),
                                                                 0.f));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("distortionType", 1),
                                                            "Compressor Speed",
                                                            juce::StringArray{"warm", "crush"}, // Choices
                                                            0 ));
    
    
    return layout;
}



float SimpleEQAudioProcessor::distortionSample(float x, float y_old, float drive, float c)
{
    const ChainSettings chainSettings = getChainSettings(apvts);
    const int distType = chainSettings.distortionType; // Ensure this exists in ChainSettings

    switch (distType)
    {
        case 0:
            return distortionCrush(x, y_old, drive, c);
        case 1:
            return distortionWarm(x, y_old, drive, c);
        default:
            return x;
    }
}



float SimpleEQAudioProcessor::distortionCrush(float x, float y_old, float drive, float c)
{
    // Compute signal envelope (RMS-based)
    static float envelope = 0.0f;
    float alpha = 0.05f; // Smoothing factor (smaller = smoother response)
    envelope = (1.0f - alpha) * envelope + alpha * std::fabs(x);

    // Apply volume-dependent gain scaling (higher volume = more saturation)
    float dynamicDrive = drive * (1.0f + 0.5f * envelope);

    // Apply soft asymmetric saturation for a warmer tone
    float saturated = std::tanh(dynamicDrive * x) * (1.2f - 0.2f * std::fabs(x));

    // Introduce mild even harmonics (tube-like behavior)
    float soft_clip = saturated + 0.5f * saturated * saturated * saturated;

    // Introduce Phase Fluttering (small variations)
//    float flutter_intensity = 0.002f + drive * 0.005f;
//    float phase_jitter = flutter_intensity * ((rand() / (float)RAND_MAX) - 0.5f);
//    soft_clip += phase_jitter;

    return soft_clip;
}


float SimpleEQAudioProcessor::distortionWarm(float x, float y_old, float drive, float c)
{
    // Compute signal envelope (RMS-based)
    static float envelope = 0.0f;
    float alpha = 0.001f; // Smoothing factor (smaller = smoother response)
    envelope = (1.0f - alpha) * envelope + alpha * std::fabs(x);

    // Apply volume-dependent gain scaling (higher volume = more saturation)
    float dynamicDrive = drive * (1.0f + 0.5f * envelope);

    // Apply soft asymmetric saturation for a warmer tone
    float saturated = std::tanh(dynamicDrive * x) * (1.2f - 0.2f * std::fabs(x));

    // Introduce mild even harmonics (tube-like behavior)
    float soft_clip = saturated + 0.5f * saturated * saturated;

    // Introduce Phase Fluttering (small variations)
//    float flutter_intensity = 0.002f + drive * 0.005f;
//    float phase_jitter = flutter_intensity * ((rand() / (float)RAND_MAX) - 0.5f);
//    soft_clip += phase_jitter;

    return soft_clip;
}





distortionSettings SimpleEQAudioProcessor::getDistortionSettings(const double intensity){
    
    distortionSettings settings;
    settings.c = 0.f;//intensity*0.8f;
    settings.drive = 1.f + 6.f*intensity;
    
    return settings;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}


