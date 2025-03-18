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
    
        

    addAndMakeVisible(quarterCircle);
    setSize (600, 400);
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
    
    int quarterCircleSize = 300; // Size of the quarter-circle component
    int xPosition = (getWidth() - quarterCircleSize) / 2;  // Center horizontally
    int yPosition = (getHeight() - quarterCircleSize) / 2; // Center vertically
    
    quarterCircle.setBounds(xPosition, yPosition, quarterCircleSize, quarterCircleSize);
    
    //float radius = quarterCircle.getRadius();
    
}

 
