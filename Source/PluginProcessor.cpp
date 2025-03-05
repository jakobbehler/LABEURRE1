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
    
    auto chainSettings = getChainSettings(apvts);
    
    // UPDATE FILTERS
    updateFilters(chainSettings);
    
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


void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //
    
    auto chainSettings = getChainSettings(apvts);
    
    // UPDATE FILTERS
    updateFilters(chainSettings);
    
    
    // creating context block by defining channels ----------------------------
    
    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    leftChain.process(leftContext);
    rightChain.process(rightContext);
    
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
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//=========================================================================================================

// PARAMETER INITIALIZATION

//=========================================================================================================


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.lowCutFreq = apvts.getRawParameterValue("lowcut_freq") ->load();
    settings.highCutFreq = apvts.getRawParameterValue("highcut_freq") ->load();
    settings.peakFreq = apvts.getRawParameterValue("peak_freq") ->load();
    settings.peakGainDecibels = apvts.getRawParameterValue("peak_gain") ->load();
    settings.peakQuality = apvts.getRawParameterValue("peak_quality") ->load();
    
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("lowcut_slope") ->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("highcut_slope") ->load());
    
    return settings;
}


// COEFFICIENT SETTING HELPERS -----------------------------------------------

void SimpleEQAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings)
{
    
    
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                chainSettings.peakFreq,
                                                                                chainSettings.peakQuality,
                                                                                juce::Decibels::decibelsToGain(chainSettings.peakGainDecibels));
    
    updateCoefficients(leftChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
}

void SimpleEQAudioProcessor::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
    *old = *replacements;
}

template<typename ChainType, typename CoefficientType> void SimpleEQAudioProcessor::updateCutFilter(ChainType& LowCut,
                                                                                                    const CoefficientType& cutCoefficients,
                                                                                                    const Slope& lowCutSlope)
{
    LowCut.template setBypassed<0>(true);
    LowCut.template setBypassed<1>(true);
    LowCut.template setBypassed<2>(true);
    LowCut.template setBypassed<3>(true);

    switch(lowCutSlope)
    {

        case Slope_12:
            *LowCut.template get<0>().coefficients = *cutCoefficients[0];
            LowCut.template setBypassed<0>(false);
        break;
        case Slope_24:
            *LowCut.template get<0>().coefficients = *cutCoefficients[0];
            LowCut.template setBypassed<0>(false);
            *LowCut.template get<1>().coefficients = *cutCoefficients[1];
            LowCut.template setBypassed<1>(false);

        break;
        case Slope_36:
            *LowCut.template get<0>().coefficients = *cutCoefficients[0];
            LowCut.template setBypassed<0>(false);
            *LowCut.template get<1>().coefficients = *cutCoefficients[1];
            LowCut.template setBypassed<1>(false);
            *LowCut.template get<2>().coefficients = *cutCoefficients[2];
            LowCut.template setBypassed<2>(false);

        break;
        case Slope_48:
            *LowCut.template get<0>().coefficients = *cutCoefficients[0];
            LowCut.template setBypassed<0>(false);
            *LowCut.template get<1>().coefficients = *cutCoefficients[1];
            LowCut.template setBypassed<1>(false);
            *LowCut.template get<2>().coefficients = *cutCoefficients[2];
            LowCut.template setBypassed<2>(false);
            *LowCut.template get<3>().coefficients = *cutCoefficients[3];
            LowCut.template setBypassed<3>(false);

        break;
    }
}


// CREATING PARAMETERS ---------------------------------------------------------


juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("lowcut_freq", 1),
                                                           "LowCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20.f));
    
    // "LowCut Freq" (ID), "LowCut Freq" (UI Name), 20.f (Min), 20000.f (Max), 1.f (Step), 0.25f (Skew factor - controls nonlinear scaling), 20.f (Default value)
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("highcut_freq", 1),
                                                           "HighCut Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
                                                           20000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("peak_freq", 1),
                                                           "Peak Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
                                                           750.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("peak_gain", 1),
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("peak_quality", 1),
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f),
                                                           1.f));
   
   //construct a string array of the different cut steepnesses (here 4 options)
    juce::StringArray stringArray;
    for (int i = 0; i<4; ++i)
    {
        juce::String str;
        str << (12 + i*12);
        str << " db/oct";
        stringArray.add(str);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("lowcut_slope", 1),"LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("highcut_slope", 1),"HighCut Slope", stringArray, 0));

    return layout;
}




void SimpleEQAudioProcessor::updateHighCut(const ChainSettings& chainSettings){
    
    auto cutCoefficientsHighcut = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq,
                                                                                                       getSampleRate(),
                                                                                                       2 * (chainSettings.highCutSlope + 1));
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();
    
    updateCutFilter(leftHighCut, cutCoefficientsHighcut, chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, cutCoefficientsHighcut, chainSettings.highCutSlope);
}


void SimpleEQAudioProcessor::updateLowCut(const ChainSettings& chainSettings){
    
    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq,
                                                                                getSampleRate(),
                                                                                2 * (chainSettings.lowCutSlope + 1));
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();
    
    updateCutFilter(leftLowCut, cutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(rightLowCut, cutCoefficients, chainSettings.lowCutSlope);
    
}

void SimpleEQAudioProcessor::updateFilters(const ChainSettings& chainSettings){
    updatePeakFilter(chainSettings);
    updateHighCut(chainSettings);
    updateLowCut(chainSettings);

}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}


