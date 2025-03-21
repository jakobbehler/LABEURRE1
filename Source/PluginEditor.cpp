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
