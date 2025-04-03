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
    auto radius = 25.f;//(float) juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f + 10.f;
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
KnobWithLabel::KnobWithLabel(const juce::String& labelText,
                             std::vector<std::pair<double, juce::String>> labelPoints)
    : label("label", labelText), snapLabels(std::move(labelPoints))
{
    label.setJustificationType(juce::Justification::centred);
    //label.setFont(14.0f);
    label.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(label);
    addAndMakeVisible(knob);
}

void KnobWithLabel::resized()
{
    auto area = getLocalBounds();
    label.setBounds(area.removeFromBottom(20)); // bottom text
    knob.setBounds(area);
}

CustomKnobComponent& KnobWithLabel::getKnob() { return knob; }

void KnobWithLabel::paint(juce::Graphics& g)
{
    if (snapLabels.empty()) return;

    auto knobBounds = knob.getBounds().toFloat();

    const float knobX = knobBounds.getX();
    const float knobWidth = knobBounds.getWidth();
    const float labelY = 5.0f;          // fixed label Y
    const float connectorY = knobBounds.getY();  // bottom of line touches knob

    g.setFont(12.0f);

    for (auto& [pos, text] : snapLabels)
    {
        float x = knobX + pos * knobWidth;

        juce::Rectangle<float> labelArea(x - 25, labelY, 50, 16);
        auto isDanger = text.containsIgnoreCase("DON'T");

        g.setColour(isDanger ? juce::Colours::orange : juce::Colours::hotpink);
        g.drawFittedText(text, labelArea.toNearestInt(), juce::Justification::centred, 1);

        g.drawLine(x, labelY + 16.0f, x, connectorY, 1.0f);
    }
}



//==============================================================================
knobSection::knobSection()
    : compressionKnob("COMPRESSION", {
        { 0.0, "GLUE" }, { 0.5, "TAME" }, { 1.0, "OTT" } }),
      saturationKnob("SATURATION", {
        { 0.0, "WARM" }, { 0.5, "CRUSH" }, { 1.0, "DON'T!" } }),
      highcutKnob("HICUT")
{
    addAndMakeVisible(compressionKnob);
    addAndMakeVisible(saturationKnob);
    addAndMakeVisible(highcutKnob);
}

knobSection::~knobSection() {}

void knobSection::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour::fromString("#FF202426"));
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colour::fromString("#FFABABAB"));
    g.drawLine(270.0f, 0.0f, 270.0f, (float)getHeight(), 1.0f);
}

void knobSection::resized()
{
    const int knobSize = 170;
    const int spacing = 205;
    const int startX = 270 + 180;
    const int centerY = getHeight() / 2 - knobSize / 2;

    compressionKnob.setBounds(startX, centerY, knobSize, knobSize);
    saturationKnob.setBounds(startX + spacing, centerY, knobSize, knobSize);
    highcutKnob.setBounds(startX + spacing * 2, centerY, knobSize, knobSize);
}


// ===============

