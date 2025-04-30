/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor (SimpleEQAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      knobSection(p)
{
    
    bg_image = juce::ImageCache::getFromMemory(BinaryData::BEURRE_BG_2_png, BinaryData::BEURRE_BG_2_pngSize);
    
    addAndMakeVisible(circle);
    
    //circle.setBounds(100, 100, 200, 200);  // adjust as needed
    addAndMakeVisible(visualizer);
    addAndMakeVisible(knobSection);
    
    addAndMakeVisible(freqLine);
    
    freqLine.toFront(true);
    
    
        
    setSize (1200, 600);
    
    startTimerHz(40);
   

    
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
        //DBG("[Editor] Sending distHighIntensity to processor: " << norm);
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
    
    
    freqLine.onYChanged = [this](float /*yPosition*/)
    {
        auto* param = audioProcessor.apvts.getParameter("bandsplit_frequency");
        float newVal = param->convertTo0to1(freqLine.getHerz());
        float currentVal = param->getValue();

        if (std::abs(currentVal - newVal) > 0.001f)
        {
            DBG("[freqLine] onYChanged fired. Updating param to Hz: " << freqLine.getHerz());
            param->setValueNotifyingHost(newVal);
        }

        syncCircleWithFreqLine();
    };

    audioProcessor.apvts.addParameterListener("bandsplit_frequency", this);
    audioProcessor.apvts.addParameterListener("distHighIntensity", this);
    audioProcessor.apvts.addParameterListener("distLowIntensity", this);
    audioProcessor.apvts.addParameterListener("compLowIntensity", this);
    audioProcessor.apvts.addParameterListener("compHighIntensity", this);





       
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
    audioProcessor.apvts.removeParameterListener("bandsplit_frequency", this);
    audioProcessor.apvts.removeParameterListener("distHighIntensity", this);
    audioProcessor.apvts.removeParameterListener("distLowIntensity", this);
    audioProcessor.apvts.removeParameterListener("compLowIntensity", this);
    audioProcessor.apvts.removeParameterListener("compHighIntensity", this);


}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::Colour hell_farb = juce::Colour::fromString("#FFF7F7F7");
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll(hell_farb);
//    g.setFont (juce::FontOptions (15.0f));
//    g.drawFittedText ("Hello!", getLocalBounds(), juce::Justification::centred, 1);
    
    g.drawImage(bg_image, getLocalBounds().toFloat());
}

void SimpleEQAudioProcessorEditor::resized()
{
    const int editorW = getWidth();
    const int editorH = getHeight();

//    const int mainAreaX = 100;
//    const int mainAreaY = 40;
//    const int mainAreaW = 1000;
//    const int mainAreaH = 350;
    
    const int mainAreaX = 0;
    const int mainAreaY = 40;
    const int mainAreaW = 1200;
    const int mainAreaH = 350;


    const int knobAreaH = 170;
    
    
    
    
    // Main visual layout
    juce::Rectangle<int> mainArea(mainAreaX, mainAreaY, mainAreaW, mainAreaH);
    
    circle.setBounds(mainArea); 
        
    // Set the visualizer to the left 100px of main area
    visualizer.setBounds(mainArea.removeFromLeft(100)); // 100×350
    

    // Frequency line over the full visual area (same as circle area)
    freqLine.setBounds(mainAreaX, mainAreaY, mainAreaW, mainAreaH);
    freqLine.setHerz(audioProcessor.apvts.getRawParameterValue("bandsplit_frequency")->load());
    //freqLine.updateYFromHerz();
    
    syncCircleWithFreqLine();

        
    //freqLine.updateYFromHerz();
    // Knob section pinned to bottom
    knobSection.setBounds(0, editorH - knobAreaH, editorW, knobAreaH);
    
    
}





void SimpleEQAudioProcessorEditor::timerCallback()
{
    auto& apvts = audioProcessor.apvts;

//    auto denormalize = [](float norm)
//    {
//        return 60.0f + norm * (170.f - 60.0f);
//    };

    // These match what's registered in the processor
//    circle.getQuad(0).setRadius(denormalize(apvts.getRawParameterValue("distHighIntensity")->load()));
//    circle.getQuad(1).setRadius(denormalize(apvts.getRawParameterValue("distLowIntensity")->load()));
//    circle.getQuad(2).setRadius(denormalize(apvts.getRawParameterValue("compLowIntensity")->load()));
//    circle.getQuad(3).setRadius(denormalize(apvts.getRawParameterValue("compHighIntensity")->load()));
//
//    freqLine.setHerz(apvts.getRawParameterValue("bandsplit_frequency")->load());
//    freqLine.updateYFromHerz();


    visualizer.setFFTData(audioProcessor.getFftData());
    

}


void SimpleEQAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    auto denormalize = [](float norm)
    {
        return 60.0f + norm * (170.f - 60.0f);
    };

    if (parameterID == "bandsplit_frequency")
    {
        freqLine.setTargetHerz(newValue);

       
//        juce::MessageManager::callAsync([this]()
//        {
//            syncCircleWithFreqLine();
//
//        });
    }
    else if (parameterID == "distHighIntensity")
    {
        circle.getQuad(0).setRadius(denormalize(newValue));
    }
    else if (parameterID == "distLowIntensity")
    {
        circle.getQuad(1).setRadius(denormalize(newValue));
    }
    else if (parameterID == "compLowIntensity")
    {
        circle.getQuad(2).setRadius(denormalize(newValue));
    }
    else if (parameterID == "compHighIntensity")
    {
        circle.getQuad(3).setRadius(denormalize(newValue));
    }
}


void SimpleEQAudioProcessorEditor::syncCircleWithFreqLine()
{
    float y = freqLine.getYposition();
    circle.setBounds(circle.getX(), y - 135, circle.getWidth(), circle.getHeight());
}
