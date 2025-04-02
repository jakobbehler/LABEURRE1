/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "QuarterCircle.h"
#include "frequencyLines.h"
#include "knobSection.h"

//struct CustomRotarySlider : juce::Slider
//{
//    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox){
//
//
//    }
//};

//==============================================================================
/**
*/



class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    SimpleEQAudioProcessor& audioProcessor;
    CircleComponent circle;
    
    
    frequencyLines visualizer;
    knobSection knobSection;
    frequencyLineComponent freqLine;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};


