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

//
//void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
//{
//    juce::ScopedNoDenormals noDenormals;
//    auto totalNumInputChannels  = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();
//
//    // In case we have more outputs than inputs, this code clears any output
//    // channels that didn't contain input data, (because these aren't
//    // guaranteed to be empty - they may contain garbage).
//    // This is here to avoid people getting screaming feedback
//    // when they first compile a plugin, but obviously you don't need to keep
//    // this code if your algorithm always overwrites all the output channels.
//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear (i, 0, buffer.getNumSamples());
//
//
//
//    updateCompressor();
//
//    // creating context block by defining channels ----------------------------
//
//    juce::dsp::AudioBlock<float> block(buffer);
//
//    auto leftBlock = block.getSingleChannelBlock(0);
//    auto rightBlock = block.getSingleChannelBlock(1);
//
//    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
//    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
//
//    leftChain.process(leftContext);
//    rightChain.process(rightContext);
//
//}
//void SimpleEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
//{
//    juce::ScopedNoDenormals noDenormals;
//
//    // 🔥 Ensure compressor updates in real-time
//    updateCompressor();
//    
//    auto totalNumInputChannels  = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();
//
//    
//    float distHigh = apvts.getRawParameterValue("distHighIntensity")->load();
//    float distLow = apvts.getRawParameterValue("distLowIntensity")->load();
//    
//    float c_high = getDistortionSettings(distHigh).c;
//    float drive_high = getDistortionSettings(distHigh).drive;
//    
//    float c_low = getDistortionSettings(distLow).c
//    float drive_low = getDistortionSettings(distLow).drive;
//    
//    
//
//    
//    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear(i, 0, buffer.getNumSamples());
//
//    float crossoverFreq = apvts.getRawParameterValue("bandsplit_frequency")->load();
//    leftChain.get<0>().setCutoffFrequency(crossoverFreq);
//    rightChain.get<0>().setCutoffFrequency(crossoverFreq);
//
//    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
//    {
//        auto* channelData = buffer.getWritePointer(channel);
//        auto& chain = (channel == 0) ? leftChain : rightChain;
//
//        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
//        {
//            float inputSample = channelData[sample];
//
//            float lowSample, highSample;
//            chain.get<0>().processSample(channel, inputSample, lowSample, highSample);
//
//            // ✅ Apply
//            lowSample = chain.get<1>().get<1>().processSample(channel, lowSample);
//            highSample = chain.get<2>().get<1>().processSample(channel, highSample);
//            
//            // ✅ Apply Gain (Makeup Compensation)
//            lowSample = chain.get<1>().get<2>().processSample(lowSample);
//            highSample = chain.get<2>().get<2>().processSample(highSample);
//
//            channelData[sample] = lowSample + highSample;
//        }
//    }
//}

void SimpleEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // 🔥 Update compressor once per block
    updateCompressor();
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Get distortion parameters for both bands
    float distHigh = apvts.getRawParameterValue("distHighIntensity")->load();
    float distLow = apvts.getRawParameterValue("distLowIntensity")->load();

    tapeDistortionSettings highSettings = getDistortionSettings(distHigh);
    tapeDistortionSettings lowSettings = getDistortionSettings(distLow);

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
            lowSample = tapeDistortionSample(lowSample, y_old_low, lowSettings.drive, lowSettings.c);
            highSample = tapeDistortionSample(highSample, y_old_high, highSettings.drive, highSettings.c);

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


void SimpleEQAudioProcessor::applyCompressorSettings(juce::dsp::Compressor<float>& compressor, juce::dsp::Gain<float>& gain, const CompressorSettings& settings)
{
    compressor.setThreshold(settings.threshold);
    compressor.setRatio(settings.ratio);
    
    gain.setGainDecibels(settings.makeupGain);
    
    compressor.setAttack(5.f);
    compressor.setRelease(150.f);
}


void SimpleEQAudioProcessor::updateCompressor()
{
    
    const ChainSettings chainSettings = getChainSettings(apvts);
    // Define compressor settings for both bands
    CompressorSettings lowBandSettings = getCompressorSettings(chainSettings.compLowIntensity);
    CompressorSettings highBandSettings = getCompressorSettings(chainSettings.compHighIntensity);

    // Apply settings to both stereo channels (Compressor + Gain)
    applyCompressorSettings(leftChain.get<1>().get<1>(), leftChain.get<1>().get<2>(), lowBandSettings);  // Low-band
    applyCompressorSettings(leftChain.get<2>().get<1>(), leftChain.get<2>().get<2>(), highBandSettings); // High-band

    applyCompressorSettings(rightChain.get<1>().get<1>(), rightChain.get<1>().get<2>(), lowBandSettings); // Low-band
    applyCompressorSettings(rightChain.get<2>().get<1>(), rightChain.get<2>().get<2>(), highBandSettings); // High-band
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
    
    
    // DISTORTION ----
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("distLowIntensity", 1),
                                                                 "distLowIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f),
                                                                 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("distHighIntensity", 1),
                                                                 "distHighIntensity",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.05f, 1.f),
                                                                 0.f));
    
    return layout;
}



float SimpleEQAudioProcessor::tapeDistortionSample(float x, float y_old, float drive, float c)
{
    // Apply drive (saturation amount) and add hysteresis feedback
    float input_signal = drive * x + c * y_old;

    // Nonlinear saturation using tanh()
    return std::tanh(input_signal) + 0.2f * std::pow(input_signal, 3);
}

tapeDistortionSettings SimpleEQAudioProcessor::getDistortionSettings(const double intensity){
    
    tapeDistortionSettings settings;
    settings.c = intensity;
    settings.drive = 1.f+ intensity * 0.5f;
    
    return settings;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}


