/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
        
    
    addAndMakeVisible(circle);
    
    //circle.setBounds(100, 100, 200, 200);  // adjust as needed
    addAndMakeVisible(visualizer);
    addAndMakeVisible(knobSection);
    addAndMakeVisible(freqLine);
    freqLine.toFront(false);
        
    setSize (1200, 600);
    
   

    
    // ======= 🔗 PARAMETER CONNECTIONS START =======

    using Param = juce::AudioProcessorValueTreeState;

    auto& apvts = audioProcessor.apvts;

  
    auto normalize = [](float radius)
    {
        return (radius - 50.0f) / (190.f - 50.0f);
    };

    auto denormalize = [](float norm)
    {
        return 50.0f + norm * (190.f - 50.0f);
    };

    // Top-right → distHigh
    auto& q0 = circle.getQuad(0);
    q0.setRadius(denormalize(apvts.getRawParameterValue("distHighIntensity")->load()));
    q0.onRadiusChanged = [this, normalize](float radius)
    {
        float norm = normalize(radius);
        auto* param = audioProcessor.apvts.getParameter("distHighIntensity");
        param->setValueNotifyingHost(norm);
        DBG("[Editor] Sending distHighIntensity to processor: " << norm);
    };

    // Bottom-right → distLow
    auto& q1 = circle.getQuad(1);
    q1.setRadius(denormalize(apvts.getRawParameterValue("distLowIntensity")->load()));
    q1.onRadiusChanged = [this, normalize](float radius)
    {
        float norm = normalize(radius);
        auto* param = audioProcessor.apvts.getParameter("distLowIntensity");
        param->setValueNotifyingHost(norm);
    };

    // Bottom-left → compLow
    auto& q2 = circle.getQuad(2);
    q2.setRadius(denormalize(apvts.getRawParameterValue("compLowIntensity")->load()));
    q2.onRadiusChanged = [this, normalize](float radius)
    {
        float norm = normalize(radius);
        auto* param = audioProcessor.apvts.getParameter("compLowIntensity");
        param->setValueNotifyingHost(norm);
    };

    // Top-left → compHigh
    auto& q3 = circle.getQuad(3);
    q3.setRadius(denormalize(apvts.getRawParameterValue("compHighIntensity")->load()));
    q3.onRadiusChanged = [this, normalize](float radius)
    {
        float norm = normalize(radius);
        auto* param = audioProcessor.apvts.getParameter("compHighIntensity");
        param->setValueNotifyingHost(norm);
    };
    
    
    freqLine.onYChanged = [this](float herz)
    {
        auto* param = audioProcessor.apvts.getParameter("bandsplit_frequency");
        param->setValueNotifyingHost(param->convertTo0to1(herz));

        float y = freqLine.getYposition(); // 🟢 query directly
        circle.setTopLeftPosition(300, y - 200);
    };


       
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::white);
//    g.setFont (juce::FontOptions (15.0f));
//    g.drawFittedText ("Hello!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    const int editorW = getWidth();
    const int editorH = getHeight();

    const int mainAreaX = 100;
    const int mainAreaY = 40;
    const int mainAreaW = 1000;
    const int mainAreaH = 350;

    const int knobAreaH = 170;

    // Main visual layout
    juce::Rectangle<int> mainArea(mainAreaX, mainAreaY, mainAreaW, mainAreaH);

    // Set the visualizer to the left 100px of main area
    visualizer.setBounds(mainArea.removeFromLeft(100)); // 100×350

    // Set the circle (the remaining 900×350)
    circle.setBounds(mainArea); // Circle will now call resized()

    // Frequency line over the full visual area (same as circle area)
    freqLine.setBounds(mainAreaX, mainAreaY, mainAreaW, mainAreaH);

    // Knob section pinned to bottom
    knobSection.setBounds(0, editorH - knobAreaH, editorW, knobAreaH);
}




void SimpleEQAudioProcessorEditor::timerCallback()
{
    auto& apvts = audioProcessor.apvts;

    auto denormalize = [](float norm)
    {
        return 50.0f + norm * (190.f - 50.0f);
    };

    // ✅ These match what's registered in the processor
    circle.getQuad(0).setRadius(denormalize(apvts.getRawParameterValue("distHighIntensity")->load()));
    circle.getQuad(1).setRadius(denormalize(apvts.getRawParameterValue("distLowIntensity")->load()));
    circle.getQuad(2).setRadius(denormalize(apvts.getRawParameterValue("compLowIntensity")->load()));
    circle.getQuad(3).setRadius(denormalize(apvts.getRawParameterValue("compHighIntensity")->load()));

    freqLine.setHerz(apvts.getRawParameterValue("bandsplit_frequency")->load());
}


