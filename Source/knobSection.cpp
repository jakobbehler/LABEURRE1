/*
  ==============================================================================

    knobSection.cpp
    Created: 2 Apr 2025 3:18:33pm
    Author:  Jakob Behler

  ==============================================================================
*/

#include <JuceHeader.h>
#include "knobSection.h"
#include "BinaryData.h"

//==============================================================================

OtherLookAndFeel::OtherLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colours::red);
}

void OtherLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    auto radius = 25.f;
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

//==============================================================================
SnapKnob::SnapKnob()
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider.setRange(0.0, 1.0, 0.01);

    slider.onValueChange = [this]() {
        double value = slider.getValue();
        if (snapLabels.empty()) return;

        double closest = snapLabels[0].first;
        for (auto& [pos, _] : snapLabels)
            if (std::abs(pos - value) < std::abs(closest - value))
                closest = pos;

        if (value != closest)
            slider.setValue(closest, juce::dontSendNotification);
    };
}

void SnapKnob::configureSnapPoints(const std::vector<std::pair<double, juce::String>>& labels,
                                   const juce::Image& image1,
                                   const juce::Image& image2,
                                   const juce::Image& image3)
{
    snapLabels = labels;
    img1 = image1;
    img2 = image2;
    img3 = image3;
}

void SnapKnob::paint(juce::Graphics& g)
{
    const float value = slider.getValue();
    const juce::Image* img = nullptr;

    if (value < 0.4f)
        img = &img1;
    else if (value < 0.6f)
        img = &img2;
    else
        img = &img3;

    if (img && img->isValid())
    {
        auto bounds = getLocalBounds().toFloat();
        float targetHeight = bounds.getHeight();
        float aspectRatio = (float)img->getWidth() / (float)img->getHeight();
        float targetWidth = targetHeight * aspectRatio;

        float x = bounds.getX() - ((targetWidth - bounds.getWidth()) / 2.0f);
        float y = bounds.getY();

        juce::Rectangle<float> targetArea(x, y, targetWidth, targetHeight);
        g.drawImage(*img, targetArea);
    }
}

//==============================================================================
knobSection::knobSection(SimpleEQAudioProcessor& proc) : processor(proc)
{
    nameImage = juce::ImageCache::getFromMemory(BinaryData::name_png, BinaryData::name_pngSize);

    
    juce::Image glue  = juce::ImageCache::getFromMemory(BinaryData::glue_png, BinaryData::glue_pngSize);
    juce::Image tame  = juce::ImageCache::getFromMemory(BinaryData::comp_png, BinaryData::comp_pngSize);
    juce::Image ott   = juce::ImageCache::getFromMemory(BinaryData::ott_png,  BinaryData::ott_pngSize);

    compressionKnob.configureSnapPoints({
        { 0.3, "GLUE" },
        { 0.5, "TAME" },
        { 0.7, "OTT" }
    }, glue, tame, ott);

    juce::Image warm   = juce::ImageCache::getFromMemory(BinaryData::WARM_png,   BinaryData::WARM_pngSize);
    juce::Image crush  = juce::ImageCache::getFromMemory(BinaryData::CRUSH_png,  BinaryData::CRUSH_pngSize);
    juce::Image dont   = juce::ImageCache::getFromMemory(BinaryData::DONT_png,   BinaryData::DONT_pngSize);

    saturationKnob.configureSnapPoints({
        { 0.3, "WARM" },
        { 0.5, "CRUSH" },
        { 0.7, "DON'T!" }
    }, warm, crush, dont);

    compressionKnob.attach(proc.apvts, "compressorSpeed");
    saturationKnob.attach(proc.apvts, "distortionType");

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

    // 👇 Draw name.png bottom-left
    if (nameImage.isValid())
    {

        int imageWidth = 270.f;
        int imageHeight = 350.f;

    

        g.drawImageWithin(nameImage, 0, 0, imageWidth, imageHeight, juce::Justification::topLeft);

    }
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
