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
    slider.setLookAndFeel(&otherLookAndFeel); // or your own look and feel if defined

    addAndMakeVisible(slider); // this actually shows it
}

CustomKnobComponent::~CustomKnobComponent()
{
    slider.setLookAndFeel(nullptr); // avoid dangling pointer
}



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
    //g.fillAll(juce::Colours::white);
    
    auto bounds = getLocalBounds();

//    // Label
//    g.setColour(juce::Colours::black);
//    g.setFont(14.0f);
//    g.drawText(name, bounds.removeFromTop(20), juce::Justification::centred);

    // Slider
    
    
    slider.setBounds(bounds);
    
    
}



//==============================================================================

OtherLookAndFeel::OtherLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colours::red);
}

void OtherLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    auto radius = (float) juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // fill
    juce::Colour hell_farb = juce::Colour::fromString("#FFABABAB");
    juce::Colour dunkel_farb = juce::Colour::fromString("#FF202426");
    
    g.setColour(hell_farb);
    g.fillEllipse(rx, ry, rw, rw);

    // pointer
    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(dunkel_farb);
    g.fillPath(p);
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
    // Add knobs to the component so they are drawn and resized
        addAndMakeVisible(compressorKnob);
        addAndMakeVisible(distortionKnob);
        addAndMakeVisible(highCutKnob);

        // Optional: look and feel (if not handled inside CustomKnobComponent already)
        compressorKnob.slider.setLookAndFeel(&otherLookAndFeel);
        distortionKnob.slider.setLookAndFeel(&otherLookAndFeel);
        highCutKnob.slider.setLookAndFeel(&otherLookAndFeel);
    
                      
}

knobSection::~knobSection()
{
}



void knobSection::paint(juce::Graphics& g)
{
    // Define your custom background color
    juce::Colour bg = juce::Colour::fromString("#FF202426");

    // Fill the component's entire bounds
    g.setColour(bg);
    g.fillRect(getLocalBounds());
    g.setColour(juce::Colour::fromString("#FFABABAB"));
    // Draw vertical line at x = 270
 
    g.drawLine(270.0f, 0.0f, 270.0f, (float)getHeight(), 1.0f);
}


void knobSection::resized()
{
    auto bounds = getLocalBounds();
    
    // Split off left label area (270 px)
    const int labelAreaWidth = 270;
    
    //bounds.removeFromLeft(labelAreaWidth); // Discard for layout
    
    
    // Knob layout values
   
    const int knobSize = 50;
    const int spacing_1= 180;
    const int spacing = 205;
 
    const int knob_start_1 = labelAreaWidth + spacing_1;
    const int knob_start_2 = knob_start_1 + knobSize + spacing;
    const int knob_start_3 = knob_start_2 + knobSize + spacing;
    
    const int verticalMidPoint = 170/2;
    const int verticalKnobStart = verticalMidPoint - knobSize/2;
    

    // Absolute x positions relative to full component
    compressorKnob.setBounds(knob_start_1, verticalKnobStart, knobSize, knobSize);
    distortionKnob.setBounds(knob_start_2, verticalKnobStart, knobSize, knobSize);
    highCutKnob.setBounds(knob_start_3, verticalKnobStart, knobSize, knobSize);
    
}


// ======


