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

    setSize (1000, 600);
    
    freqLine.onYChanged = [this](float y)
    {
        circle.setTopLeftPosition(300, y - 200);
    };

    
    // ======= 🔗 PARAMETER CONNECTIONS START =======

    using Param = juce::AudioProcessorValueTreeState;

    auto& apvts = audioProcessor.apvts;

    auto& q0 = circle.getQuad(0); // Top-right → distHigh
    q0.setRadius(apvts.getRawParameterValue("distHighIntensity")->load());
    q0.onRadiusChanged = [this](float val)
    {
        DBG("[Editor] Sending distHighIntensity to processor: " << val);
        auto* param = audioProcessor.apvts.getParameter("distHighIntensity");
        param->setValueNotifyingHost(param->convertTo0to1(val));
    };

    auto& q1 = circle.getQuad(1); // Bottom-right → distLow
    q1.setRadius(apvts.getRawParameterValue("distLowIntensity")->load());
    q1.onRadiusChanged = [this](float val)
    {
        auto* param = audioProcessor.apvts.getParameter("distLowIntensity");
        param->setValueNotifyingHost(param->convertTo0to1(val));
    };

    auto& q2 = circle.getQuad(2); // Bottom-left → compLow
    q2.setRadius(apvts.getRawParameterValue("compLowIntensity")->load());
    q2.onRadiusChanged = [this](float val)
    {
        auto* param = audioProcessor.apvts.getParameter("compLowIntensity");
        param->setValueNotifyingHost(param->convertTo0to1(val));
    };

    auto& q3 = circle.getQuad(3); // Top-left → compHigh
    q3.setRadius(apvts.getRawParameterValue("compHighIntensity")->load());
    q3.onRadiusChanged = [this](float val)
    {
        auto* param = audioProcessor.apvts.getParameter("compHighIntensity");
        param->setValueNotifyingHost(param->convertTo0to1(val));
    };


       
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
//
//    g.setColour (juce::Colours::white);
//    g.setFont (juce::FontOptions (15.0f));
//    g.drawFittedText ("Hello!", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
 
    

    //float radius = quarterCircle.getRadius();
    freqLine.setBounds(0, 0, getWidth(), getHeight());

    float lineYpos = freqLine.getYposition();
    
    circle.setBounds(300, 200, 400, 400); // Circle now aligns correctly
    
}

void SimpleEQAudioProcessorEditor::timerCallback()
{
    auto& apvts = audioProcessor.apvts;

    circle.getQuad(0).setRadius(apvts.getRawParameterValue("distortionTop")->load());
    circle.getQuad(1).setRadius(apvts.getRawParameterValue("distortionLow")->load());
    circle.getQuad(2).setRadius(apvts.getRawParameterValue("compressionBottom")->load());
    circle.getQuad(3).setRadius(apvts.getRawParameterValue("compressionTop")->load());
}

