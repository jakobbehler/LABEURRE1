/*
  ==============================================================================

    knobSection.cpp
    Created: 2 Apr 2025 3:18:33pm
    Author:  Jakob Behler

  ==============================================================================
*/

#include <JuceHeader.h>
#include "knobSection.h"



//==============================================================================


CustomKnobComponent::CustomKnobComponent()
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(slider);
}

CustomKnobComponent::~CustomKnobComponent() = default;


void CustomKnobComponent::resized()
{
    slider.setBounds(getLocalBounds());
}

void CustomKnobComponent::attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
}



void CustomKnobComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    
    auto bounds = getLocalBounds();

    // Label
    g.setColour(juce::Colours::black);
    g.setFont(14.0f);
    g.drawText(name, bounds.removeFromTop(20), juce::Justification::centred);

    // Slider
    auto sliderBounds = bounds.reduced(10);
    slider.setBounds(sliderBounds);
}


//==============================================================================


CompressionKnob::CompressionKnob()
{
    name = "COMP";
    labels = {
        { 0.0, "WARM" },
        { 1.0, "CRUSH" }
    };
    shouldSnapToLabels = true;

}

double CompressionKnob::snapValue(double value, juce::Slider::DragMode)
{
    const std::vector<double> points = { 0.0, 0.5, 1.0 };
    double closest = points[0];
    for (auto p : points)
        if (std::abs(value - p) < std::abs(value - closest))
            closest = p;

    return closest;
}


//==============================================================================

SaturationKnob::SaturationKnob()
{
    name = "SAT";
    labels = {
        { 0.0, "WARM" },
        { 0.5, "CRUSH" },
        { 1.0, "DON'T!" }
    };
}

double SaturationKnob::snapValue(double value, juce::Slider::DragMode)
{
    const std::vector<double> points = { 0.0, 0.5, 1.0 };
    double closest = points[0];
    for (auto p : points)
        if (std::abs(value - p) < std::abs(value - closest))
            closest = p;

    return closest;
}

//==============================================================================



HicutKnob::HicutKnob()
{
    name = "HICUT";
    shouldSnapToLabels = false;
}
//==============================================================================
// PARENT
//==============================================================================

knobSection::knobSection()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(compressorKnob);
    addAndMakeVisible(distortionKnob);
    addAndMakeVisible(highCutKnob);
    
                      
}

knobSection::~knobSection()
{
}

void knobSection::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("knobSection", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}



void knobSection::resized()
{
    auto bounds = getLocalBounds();
    
    // Split off left label area (270 px)
    const int labelAreaWidth = 270;
    
    //bounds.removeFromLeft(labelAreaWidth); // Discard for layout
    
    
    // Knob layout values
   
    const int knobSize = 50;
    const int spacing1 = 225;
    const int spacing2 = 130;
    const int spacing3 = 130;
    
    const int verticalMidPoint = 170/2;
    const int verticalKnobStart = verticalMidPoint - knobSize/2;
    

    // Absolute x positions relative to full component
    compressorKnob.setBounds(labelAreaWidth + spacing1, verticalKnobStart, knobSize, knobSize);
    distortionKnob.setBounds(labelAreaWidth + spacing1 + spacing2 + knobSize, verticalKnobStart, knobSize, knobSize);
    highCutKnob.setBounds(labelAreaWidth + spacing1 + spacing2 + spacing3 + 2*knobSize, verticalKnobStart, knobSize, knobSize);
}
