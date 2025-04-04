#pragma once

#include <JuceHeader.h>

//==============================================================================
class OtherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OtherLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
};

//==============================================================================
class CustomKnobComponent : public juce::Component
{
public:
    CustomKnobComponent();
    ~CustomKnobComponent() override;

    //void paint(juce::Graphics&) override;
    void resized() override;

    void attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID);

protected:
    juce::Slider slider;
    OtherLookAndFeel otherLookAndFeel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

//==============================================================================
class SnapKnob : public CustomKnobComponent
{
public:
    SnapKnob();

    void configureSnapPoints(const std::vector<std::pair<double, juce::String>>& labels,
                             const juce::Image& img1,
                             const juce::Image& img2,
                             const juce::Image& img3);
    
    void paint(juce::Graphics& g) override;

private:
    std::vector<std::pair<double, juce::String>> snapLabels;
    juce::Image img1, img2, img3;
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
    SnapKnob compressionKnob;
    SnapKnob saturationKnob;
    CustomKnobComponent highcutKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(knobSection)
};
