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



class SimpleEQAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer, private juce::AudioProcessorValueTreeState::Listener
{
public:
    SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor&);
    ~SimpleEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void syncCircleWithFreqLine();
    
    //void addMouseListenerToAll(juce::Component* listener);
    void mouseDown(const juce::MouseEvent&) override { setMouseCursor(clickCursor); }
    void mouseUp(const juce::MouseEvent&) override   { setMouseCursor(normalCursor); }


    
    void addMouseListenerToChildren(juce::MouseListener* listener);
    
    void applyCursorToAllChildren(juce::Component& parent);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    std::vector<float> fftBins;

    juce::Image bg_image;
    
    juce::MouseCursor normalCursor;
    juce::MouseCursor clickCursor;

    
    SimpleEQAudioProcessor& audioProcessor;
    CircleComponent circle;
    
    
    frequencyLines visualizer;
    knobSection knobSection;
    frequencyLineComponent freqLine;
    
    // GUI listener!!!
    void parameterChanged(const juce::String& parameterID, float newValue) override;
        
    

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessorEditor)
};



