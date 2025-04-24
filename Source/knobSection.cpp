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

    
    auto thumbColour = slider.findColour (juce::Slider::thumbColourId);
    
    g.setColour(thumbColour);
    g.fillEllipse(rx, ry, rw, rw);

    juce::Path p;
    auto pointerLength = radius * 0.7f;
    auto pointerThickness = 2.0f;
    
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    
    juce::Colour pointerColour = juce::Colour::fromString("#FFF7F7F7");
    
    if (thumbColour.getPerceivedBrightness() > 0.85f)
        pointerColour = juce::Colour::fromString("#FF6184FF");
 
    g.setColour (pointerColour);
    g.fillPath (p);
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

void CustomKnobComponent::paint(juce::Graphics& g)
{
    if (backgroundImage.isValid())
    {
        auto bounds = getLocalBounds().toFloat();
        
        float aspectRatio = (float)backgroundImage.getWidth() / (float)backgroundImage.getHeight();
        float targetHeight = bounds.getHeight();
        float targetWidth = targetHeight * aspectRatio;

        // Clamp width to bounds
        if (targetWidth > bounds.getWidth())
        {
            targetWidth = bounds.getWidth();
            targetHeight = targetWidth / aspectRatio;
        }

        float x = bounds.getCentreX() - targetWidth / 2.0f;
        float y = bounds.getCentreY() - targetHeight / 2.0f;

        g.drawImage(backgroundImage, x, y, targetWidth, targetHeight, 0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
    }
}


void CustomKnobComponent::attach(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
}

void CustomKnobComponent::setBackgroundImage(const juce::Image& img)
{
    backgroundImage = img;
}

void CustomKnobComponent::setThumbColour (juce::Colour c)
{
    slider.setColour (juce::Slider::thumbColourId, c);
}


//==============================================================================
SnapKnob::SnapKnob()
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    slider.setRange (0.0, 1.0, 0.01);

    slider.onValueChange = [this]
    {
        if (snapLabels.empty()) return;

        // ----- 1. find nearest snap-point ----------------
        double v = slider.getValue();
        targetValue = snapLabels.front().first;

        for (auto& [p, _] : snapLabels)
            if (std::abs (p - v) < std::abs (targetValue - v))
                targetValue = p;

        // ----- 2. start interpolation if not already there
        if (std::abs (v - targetValue) > 0.001)
            startTimerHz (60);
    };
}

void SnapKnob::timerCallback()
{
    constexpr double smoothing = 0.30;              // 0…1  (smaller = slower)

    double v   = slider.getValue();
    double diff= targetValue - v;

    if (std::abs (diff) < 0.00005)                   // close enough
    {
        slider.setValue (targetValue, juce::dontSendNotification);
        stopTimer();
    }
    else
    {
        slider.setValue (v + diff * smoothing, juce::dontSendNotification);
    }
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

        // Clamp image width to not exceed bounds
        if (targetWidth > bounds.getWidth())
        {
            targetWidth = bounds.getWidth();
            targetHeight = targetWidth / aspectRatio;
        }

        float x = bounds.getCentreX() - targetWidth / 2.0f;
        float y = bounds.getCentreY() - targetHeight / 2.0f;

        g.drawImage(*img, x, y, targetWidth, targetHeight, 0, 0, img->getWidth(), img->getHeight());
    }
}


//==============================================================================
knobSection::knobSection(SimpleEQAudioProcessor& proc) : processor(proc)
{
    //nameImage = juce::ImageCache::getFromMemory(BinaryData::name_png, BinaryData::name_pngSize);

    
    juce::Image glue  = juce::ImageCache::getFromMemory(BinaryData::glue_png, BinaryData::glue_pngSize);
    juce::Image tame  = juce::ImageCache::getFromMemory(BinaryData::tame_png, BinaryData::tame_pngSize);
    juce::Image ott   = juce::ImageCache::getFromMemory(BinaryData::ott_png,  BinaryData::ott_pngSize);

    compressionKnob.configureSnapPoints({
        { 0.3, "GLUE" },
        { 0.5, "TAME" },
        { 0.7, "OTT" }
    }, glue, tame, ott);

    juce::Image warm   = juce::ImageCache::getFromMemory(BinaryData::warm_png,   BinaryData::warm_pngSize);
    juce::Image crush  = juce::ImageCache::getFromMemory(BinaryData::crush_png,  BinaryData::crush_pngSize);
    juce::Image dont   = juce::ImageCache::getFromMemory(BinaryData::dont_png,   BinaryData::dont_pngSize);

    saturationKnob.configureSnapPoints({
        { 0.3, "WARM" },
        { 0.5, "CRUSH" },
        { 0.7, "DON'T!" }
    }, warm, crush, dont);
    
    juce::Image hiCutImg = juce::ImageCache::getFromMemory(BinaryData::hicut_png, BinaryData::hicut_pngSize);
    highcutKnob.setBackgroundImage(hiCutImg);
    
    // start and end point for hicut slide
    const float startAngle = juce::MathConstants<float>::pi * 1.7f;
    const float endAngle   = startAngle + juce::MathConstants<float>::pi * 0.85f;

    highcutKnob.slider.setRotaryParameters (startAngle, endAngle, true);

    
    
    
    juce::Colour blau = juce::Colour::fromString("#FF6184FF");
    juce::Colour rot = juce::Colour::fromString("#FFF0597C");
    juce::Colour white = juce::Colour::fromString("#FFF7F7F7");
    
    compressionKnob.setThumbColour (blau);
    saturationKnob.setThumbColour  (rot);
    highcutKnob  .setThumbColour  (white); // or blue/red etc.

    
    compressionKnob.attach(proc.apvts, "compressorSpeed");
    saturationKnob.attach(proc.apvts, "distortionType");
    highcutKnob.attach(proc.apvts, "highCutFreq");


    addAndMakeVisible(compressionKnob);
    addAndMakeVisible(saturationKnob);
    addAndMakeVisible(highcutKnob);
    
}

knobSection::~knobSection() {}

void knobSection::paint(juce::Graphics& g)
{
    //g.setColour(juce::Colour::fromString("#FF202426"));
    //g.fillRect(getLocalBounds());

    //g.setColour(juce::Colour::fromString("#FFABABAB"));
    //g.drawLine(270.0f, 0.0f, 270.0f, (float)getHeight(), 1.0f);

//    if (nameImage.isValid())
//    {
//
//        int imageWidth = 270.f;
//        int imageHeight = 350.f;
//
//
//
//        g.drawImageWithin(nameImage, 0, 0, imageWidth, imageHeight, juce::Justification::topLeft);
//
//    }
}

void knobSection::resized()
{
    const int knobComponentWidth = 175; // 180px left + 50px knob + 180px right
    const int knobHeight = 320;         // same height so background image stays proportional
    const int spacing1 = 83;
    const int spacing2 = 89;
    
    // no extra spacing needed between components
    
    const int startX = 387;             // account for name image on left
    const int centerY = getHeight() / 2 - knobHeight / 2;

    compressionKnob.setBounds(startX, centerY, knobComponentWidth, knobHeight);
    saturationKnob.setBounds(startX + knobComponentWidth + spacing1, centerY, knobComponentWidth, knobHeight);
    highcutKnob.setBounds(startX + knobComponentWidth*2+ spacing1 + spacing2, centerY, knobComponentWidth, knobHeight);
}
