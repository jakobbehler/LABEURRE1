/*
  ==============================================================================
   frequencyLines.cpp
   Updated: 24 Apr 2025 – skip the *four* lowest‑frequency FFT bins (those
   rendered at the very bottom / highest‑Y position) and draw exactly 16 bars.
   Author: Jakob Behler
  ==============================================================================
*/
#include <JuceHeader.h>
#include "frequencyLines.h"

namespace
{
    // Number of lowest‑frequency bins we completely ignore (visual bottom rows)
    constexpr int kSkippedLowBins = 10;
    // Number of bars we want to see after skipping
    constexpr int kVisibleBars   = 18;
}

//==============================================================================
frequencyLines::frequencyLines()
{
    startTimerHz (35);   // UI update rate
}

frequencyLines::~frequencyLines() = default;

//==============================================================================
void frequencyLines::paint (juce::Graphics& g)
{
    // Make sure we have enough data to draw the requested bins
    if (static_cast<int> (smoothedBins.size()) < kSkippedLowBins + kVisibleBars)
        return;

    const auto bounds = getLocalBounds().toFloat();
    const float height = bounds.getHeight();
    const float width  = bounds.getWidth();
    const float rowH   = height / static_cast<float> (kVisibleBars - 1);

    // We iterate bottom → top.  i = 0 is the first visible row above the skipped
    // bins; its FFT index is (kSkippedLowBins).
    for (int i = 0; i < kVisibleBars; ++i)
    {
        const int binIdx   = kSkippedLowBins + i;   // Offset past skipped bins
        const float magDB  = smoothedBins[binIdx];  // FFT magnitude in dB

        // Clamp, normalise and perceptually scale  ——> 0 … 1 range
        const float clamped = juce::jlimit (-80.0f, 0.0f, magDB);
        float norm          = juce::jmap (clamped, -70.0f, -10.0f, 0.0f, 1.0f);
        norm                = juce::jlimit (0.0f, 1.0f, norm);
        norm                = std::pow (norm,  1.1f);

        const float lineLen = 15.0f + juce::jmap (norm, 0.0f, 1.0f, 0.0f, width - 15.0f);
        const float y       = (kVisibleBars - 1 - i) * rowH; // bottom‑to‑top visual layout

        g.setColour (juce::Colour::fromString ("#FFF7F7F7"));
        g.drawLine (0.0f, y, lineLen, y, 1.0f);
    }
}

//==============================================================================
void frequencyLines::resized() {}

//==============================================================================
void frequencyLines::setFFTData (const std::vector<float>& newFFTData)
{
    constexpr float smoothing = 0.9f;

    if (smoothedBins.size() != newFFTData.size())
        smoothedBins = newFFTData; // initialise on first call or size change

    for (size_t i = 0; i < newFFTData.size(); ++i)
        smoothedBins[i] = smoothing * smoothedBins[i] + (1.0f - smoothing) * newFFTData[i];

    fftBins = newFFTData; // keep raw copy if needed elsewhere
}

//==============================================================================
void frequencyLines::timerCallback()
{
    repaint();
}
