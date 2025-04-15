/*
  ==============================================================================

    frequencyLines.h
    Created: 2 Apr 2025 3:05:21pm
    Author:  Jakob Behler

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class frequencyLines : public juce::Component, private juce::Timer
{
public:
    frequencyLines();
    
    ~frequencyLines() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void setFFTData(const std::vector<float>& newFFTData);

private:
    void timerCallback() override;

    std::vector<float> fftBins;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(frequencyLines)
};
