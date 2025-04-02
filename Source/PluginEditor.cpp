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
    addAndMakeVisible(freqLine);
    //circle.setBounds(100, 100, 200, 200);  // adjust as needed
    addAndMakeVisible(visualizer);
    setSize (1000, 600);
    
   

    
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
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    
 
    

    //float radius = quarterCircle.getRadius();
    freqLine.setBounds(0, 0, getWidth(), getHeight());
    circle.setBounds(300, 200, 400, 400); // Circle now aligns correctly
    visualizer.setBounds(0, 0, 50, getHeight());
    
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


