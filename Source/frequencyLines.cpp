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
        startTimerHz(60); // update 60 times per second
    }
    

frequencyLines::~frequencyLines()
{
}

void frequencyLines::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    // Draw background scale lines (keep your existing logic)
    g.setColour(juce::Colours::darkgrey);
    const int numTicks = 10;
    for (int i = 0; i < numTicks; ++i)
    {
        float norm = static_cast<float>(i) / (numTicks - 1);
        float y = juce::jmap(1.0f - norm, 0.0f, (float)getHeight());
        g.drawLine(0.0f, y, 15.0f, y, 1.0f);
    }

    // Draw FFT bins
    if (fftBins.empty()) return;

    g.setColour(juce::Colours::aqua);
    auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    const int numBins = fftBins.size();
    const float binWidth = width / (float)numBins;

    for (int i = 0; i < numBins; ++i)
    {
        float binHeight = juce::jmap(fftBins[i], -100.0f, 0.0f, 0.0f, height);
        g.drawLine(i * binWidth, height, i * binWidth, height - binHeight);
    }
}


void frequencyLines::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}




void frequencyLines::setFFTData(const std::vector<float>& newFFTData)
{
    fftBins = newFFTData;
}

void frequencyLines::timerCallback()
{
    repaint(); // repaint yourself
}
