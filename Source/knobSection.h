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

//==============================================================================
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
    juce::String name;
    OtherLookAndFeel otherLookAndFeel;
    bool shouldSnapToLabels = false;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

//==============================================================================
class KnobWithLabel : public juce::Component
{
public:
    KnobWithLabel(const juce::String& labelText);

    void resized() override;

    CustomKnobComponent& getKnob();

private:
    juce::Label label;
    CustomKnobComponent knob;
};

//==============================================================================
class knobSection : public juce::Component
{
public:
    knobSection();
    ~knobSection() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    KnobWithLabel compressorKnob;
    KnobWithLabel distortionKnob;
    KnobWithLabel highCutKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(knobSection)
};

// ------

