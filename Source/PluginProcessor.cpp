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
    
    // DEBUG
    
    static float lastFreq = -1.0f; // cache last value
    if (std::abs(lastFreq - crossoverFreq) > 0.01f) // only log changes
    {
        DBG("🔀 Band Split Frequency updated: " << crossoverFreq << " Hz");
        lastFreq = crossoverFreq;
    }

    //---
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
    return new SimpleEQAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
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
    
    float normalized = juce::jlimit(0.0, 1.0, intensity);
    settings.makeupGain = std::pow(1.0f + normalized * 2.5f, 2.5f);  // e.g. ranges from ~1.0 to ~5.7
    
    return settings;
    
}


void SimpleEQAudioProcessor::applyCompressorSettings(juce::dsp::Compressor<float>& compressor, juce::dsp::Gain<float>& gain, const CompressorSettings& settings, int compressorSpeed)
{
    compressor.setThreshold(settings.threshold);
    compressor.setRatio(settings.ratio);
    
    gain.setGainDecibels(settings.makeupGain);
    
    float fastestAttack = 1000.0f / getSampleRate();
    
    // Attack & Release settings based on compressor speed (0 = Fast, 1 = Slow)
    const std::array<float, 3> attackTimes = {fastestAttack, 100.0f, fastestAttack};  // ms
    const std::array<float, 3> releaseTimes = {50.0f, 200.0f, 60.0f}; // ms

    compressor.setAttack(attackTimes[compressorSpeed]);
    compressor.setRelease(releaseTimes[compressorSpeed]);
    
}


void SimpleEQAudioProcessor::updateCompressor()
{
    const ChainSettings chainSettings = getChainSettings(apvts);

    // Get compressor float value from APVTS
    float compSpeedRaw = apvts.getRawParameterValue("compressorSpeed")->load();

    // 🧠 Map exact float snap values to int states for processing
    int compSpeed;
    if      (compSpeedRaw < 0.4f) compSpeed = 0; // GLUE
    else if (compSpeedRaw < 0.6f) compSpeed = 1; // TAME
    else                          compSpeed = 2; // OTT

    // Define compressor settings for both bands
    CompressorSettings lowBandSettings = getCompressorSettings(chainSettings.compLowIntensity);
    CompressorSettings highBandSettings = getCompressorSettings(chainSettings.compHighIntensity);

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
    
//    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("compressorSpeed", 1),
//                                                            "Compressor Speed",
//                                                            juce::StringArray{"Fast", "Slow", "OTT"}, // Choices
//                                                            0 ));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compressorSpeed", 1),
        "Compressor Speed",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.3f));
                                               
    
    // DISTORTION ----
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("distLowIntensity", 1),
                                                                 "distLowIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 0.75f),
                                                                 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("distHighIntensity", 1),
                                                                 "distHighIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 0.75f),
                                                                 0.f));
    
//    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("distortionType", 1),
//                                                            "Compressor Speed",
//                                                            juce::StringArray{"warm", "crush", "dont"}, // Choices
//                                                            0 ));
//
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("distortionType", 1),
        "Distortion Type",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.3f));

        

    return layout;
}



float SimpleEQAudioProcessor::distortionSample(float x, float y_old, float drive, float c)
{
    float distRaw = apvts.getRawParameterValue("distortionType")->load();
    int distType;

    if      (distRaw < 0.4f) distType = 0; // WARM
    else if (distRaw < 0.6f) distType = 1; // CRUSH
    else                    distType = 2; // DON'T!

    switch (distType)
    {
        case 0:
            return distortionWarm(x, y_old, drive, c);
        case 1:
            return distortionCrush(x, y_old, drive, c);
        case 2:
            return distortionDONT(x, y_old, drive, c);  // implement this if needed
        default:
            return x;
    }
}


float SimpleEQAudioProcessor::asymmetricSoftClip(float x, float posThreshold, float negThreshold)
{
    if (x > posThreshold) return posThreshold - (x - posThreshold) / (1 + (x - posThreshold));
    if (x < negThreshold) return negThreshold - (x - negThreshold) / (1 + (x - negThreshold));
    return x;
}



float SimpleEQAudioProcessor::distortionWarm(float x, float y_old, float drive, float c)
{
 
    // Soft clipping function (prevents hard digital clipping)
    float softClip = std::tanh(drive * x);

    // Harmonic Enhancement: Adds gentle tube-style warmth
    softClip += 0.15f * softClip * softClip * softClip;
    
    float scale = 1.0f / (1.0f + (0.295f) * (drive - 1.0f));
    return scale * softClip;
}


float SimpleEQAudioProcessor::distortionCrush(float x, float y_old, float drive, float c)
{
//    // Compute signal envelope (RMS-based)
    static float envelope = 0.0f;
    float alpha = 0.001f; // Smoothing factor (smaller = smoother response)
    envelope = (1.0f - alpha) * envelope + alpha * std::fabs(x);

    // Apply volume-dependent gain scaling (higher volume = more saturation)
    float dynamicDrive = drive * (1.0f + 0.5f * envelope);
    
    float scale = 1.0f / (1.0f + (0.3f) * (drive - 1.0f));
    // Apply soft asymmetric saturation for a warmer tone
    
    float warm = distortionWarm( x, y_old,  dynamicDrive,  c);
    
    float saturated =scale *std::tanh(dynamicDrive * warm);
    

    return saturated;
}

float SimpleEQAudioProcessor::distortionDONT(float x, float y_old, float drive, float c)
{
    static float envelope = 0.0f;
    float alpha = 0.001f;
    envelope = (1.0f - alpha) * envelope + alpha * std::fabs(x);

    float dynamicDrive = drive * drive * (1.0f + 0.5f * envelope);
    float scale = 1.0f / (1.0f + 0.3f * (drive - 1.0f));

    float warm = distortionWarm(x, y_old, dynamicDrive, c);
    float saturated = scale * std::tanh(dynamicDrive * warm);
    float saturated2 = std::tanh(dynamicDrive * dynamicDrive * saturated);

    // 🔻 Final scaling: As drive increases, output is attenuated more
    float gainCompensation = juce::jmap(drive, 1.0f, 7.0f, 0.5f, 0.15f);  // from 1.0 to 0.5


    gainCompensation = std::pow(gainCompensation, 1.1f);  // Optional tweak

    return saturated2 * gainCompensation;
}






distortionSettings SimpleEQAudioProcessor::getDistortionSettings(const double intensity){
    
    distortionSettings settings;
    settings.c = 0.2;
    settings.drive = 1.f + 6.f*intensity;
    
    return settings;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}


