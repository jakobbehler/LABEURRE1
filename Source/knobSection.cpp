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

    juce::Colour hell_farb = juce::Colour::fromString("#FFABABAB");
    juce::Colour dunkel_farb = juce::Colour::fromString("#FF202426");

    g.setColour(hell_farb);
    g.fillEllipse(rx, ry, rw, rw);

    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(dunkel_farb);
    g.fillPath(p);
}

//==============================================================================
CustomKnobComponent::CustomKnobComponent()
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider.setLookAndFeel(&otherLookAndFeel);
    addAndMakeVisible(slider);
}

CustomKnobComponent::~CustomKnobComponent()
{
    slider.setLookAndFeel(nullptr);
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
    // nothing for now
}

//==============================================================================
KnobWithLabel::KnobWithLabel(const juce::String& labelText)
    : label("label", labelText)
{
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
    addAndMakeVisible(knob);
}

void KnobWithLabel::resized()
{
    auto area = getLocalBounds();
    label.setBounds(area.removeFromBottom(20));
    knob.setBounds(area);
}

CustomKnobComponent& KnobWithLabel::getKnob() { return knob; }

//==============================================================================
knobSection::knobSection()
    : compressorKnob("COMP"), distortionKnob("SAT"), highCutKnob("HICUT")
{
    addAndMakeVisible(compressorKnob);
    addAndMakeVisible(distortionKnob);
    addAndMakeVisible(highCutKnob);
}

knobSection::~knobSection() {}

void knobSection::paint(juce::Graphics& g)
{
    juce::Colour bg = juce::Colour::fromString("#FF202426");
    g.setColour(bg);
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colour::fromString("#FFABABAB"));
    g.drawLine(270.0f, 0.0f, 270.0f, (float)getHeight(), 1.0f);
}

void knobSection::resized()
{
    auto bounds = getLocalBounds();
    const int labelAreaWidth = 270;

    const int knobSize = 50;
    const int spacing_1 = 180;
    const int spacing = 205;

    const int knob_start_1 = labelAreaWidth + spacing_1;
    const int knob_start_2 = knob_start_1 + knobSize + spacing;
    const int knob_start_3 = knob_start_2 + knobSize + spacing;

    const int verticalMidPoint = 170 / 2;
    const int verticalKnobStart = verticalMidPoint - knobSize / 2;

    compressorKnob.setBounds(knob_start_1, verticalKnobStart, knobSize, knobSize + 20);
    distortionKnob.setBounds(knob_start_2, verticalKnobStart, knobSize, knobSize + 20);
    highCutKnob.setBounds(knob_start_3, verticalKnobStart, knobSize, knobSize + 20);
}
