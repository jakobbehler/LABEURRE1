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
    startTimerHz(25); // update rate
}

frequencyLines::~frequencyLines()
{
}

void frequencyLines::paint(juce::Graphics& g)
{
    

    const int numTicks = 20;

    if ((int)smoothedBins.size() < numTicks)
        return;

    auto bounds = getLocalBounds().toFloat();
    float height = bounds.getHeight();
    float width = bounds.getWidth();
    float binHeight = height / (float)(numTicks - 1);

    for (int i = 0; i < numTicks; ++i)
    {
        float magnitude = smoothedBins[i];

        float clampedMag = juce::jlimit(-80.0f, 0.0f, magnitude);

        float norm = juce::jmap(clampedMag, -70.0f, -10.0f, 0.0f, 1.0f);
        norm = juce::jlimit(0.0f, 1.0f, norm);

        norm = std::pow(norm, 2.5f);

        float lineLength = 15.f + juce::jmap(norm, 0.f, 1.f, 0.f, width-15.f);
        

        float y = (numTicks - 1 - i) * binHeight;
        
        juce::Colour hell_farb = juce::Colour::fromString("#FFF7F7F7");
        
        g.setColour(hell_farb);
        g.drawLine(0.0f, y, lineLength, y, 1.0f);
    }
}

void frequencyLines::resized()
{
    // No child components yet
}

void frequencyLines::setFFTData(const std::vector<float>& newFFTData)
{
    const float smoothingFactor = 0.9f;

    if (smoothedBins.size() != newFFTData.size())
        smoothedBins = newFFTData;

    for (size_t i = 0; i < newFFTData.size(); ++i)
    {
        smoothedBins[i] = smoothingFactor * smoothedBins[i] + (1.0f - smoothingFactor) * newFFTData[i];
    }

    fftBins = newFFTData; // still store raw bins if needed
}

void frequencyLines::timerCallback()
{
    repaint();
}
