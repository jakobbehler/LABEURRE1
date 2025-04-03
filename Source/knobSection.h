/*
  ==============================================================================

    knobSection.h
    Created: 2 Apr 2025 3:18:33pm
    Author:  Jakob Behler

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/



class CustomKnobComponent : public juce::Component
{
public:
    CustomKnobComponent();
    ~CustomKnobComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);

protected:
    

    juce::String name;
    
    bool shouldSnapToLabels = false;

    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};



class CompressionKnob : public CustomKnobComponent
{
public:
    CompressionKnob();
    double snapValue(double value, juce::Slider::DragMode);
protected:
    std::vector<std::pair<double, juce::String>> labels;
};



class SaturationKnob : public CustomKnobComponent
{
public:
    SaturationKnob();
    double snapValue(double value, juce::Slider::DragMode);
protected:
    std::vector<std::pair<double, juce::String>> labels;
};


class HicutKnob : public CustomKnobComponent
{
public:
    HicutKnob();
};


class knobSection  : public juce::Component
{
public:
    knobSection();
    ~knobSection() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CompressionKnob compressorKnob;
    SaturationKnob distortionKnob;
    HicutKnob highCutKnob;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (knobSection)
    
};
