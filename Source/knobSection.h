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


class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;
};


class CustomKnobComponent : public juce::Component
{
public:
    CustomKnobComponent();
    ~CustomKnobComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);
    juce::Slider slider;

protected:
    
    OtherLookAndFeel otherLookAndFeel;

    juce::String name;
    
    
    bool shouldSnapToLabels = false;

    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// ----------------------------------------------------------------------

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



// ----------------------------------------------------------------------
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
    
    OtherLookAndFeel otherLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (knobSection)
    
};


// ------

