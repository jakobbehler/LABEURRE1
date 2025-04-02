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
class frequencyLines  : public juce::Component
{
public:
    frequencyLines();
    ~frequencyLines() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (frequencyLines)
};
