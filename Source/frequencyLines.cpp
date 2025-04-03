/*
  ==============================================================================

    frequencyLines.cpp
    Created: 2 Apr 2025 3:05:21pm
    Author:  Jakob Behler

  ==============================================================================
*/

#include <JuceHeader.h>
#include "frequencyLines.h"

//==============================================================================
frequencyLines::frequencyLines()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

frequencyLines::~frequencyLines()
{
}

void frequencyLines::paint(juce::Graphics& g)
    {
        g.fillAll(juce::Colours::transparentBlack);

        // Draw scale lines
        //juce::Font labelFont(12.0f);
        g.setColour(juce::Colours::black);
        //g.setFont(labelFont);

        const int numTicks = 20;
        const float minHz = 20.0f;
        const float maxHz = 20000.0f;
        const float minY = 0;
        const float maxY = 350;

        for (int i = 0; i < numTicks; ++i)
        {
            float norm = static_cast<float>(i) / (numTicks - 1);
            //float logHz = std::pow(10.0f, juce::jmap(norm, std::log10(minHz), std::log10(maxHz)));
            float y = juce::jmap(1.0f - norm, minY, maxY); // flipped Y axis

            g.drawLine(0.0f, y, 15.0f, y, 1.0f);
            //g.drawText(juce::String(static_cast<int>(logHz)) + " Hz",
                       //20, static_cast<int>(y - 6), 60, 12, juce::Justification::left);
        }
    }

void frequencyLines::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
