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


//==============================================================================
frequencyLines::frequencyLines()
{
    startTimerHz (35);   // UI update rate
}

frequencyLines::~frequencyLines() = default;

//==============================================================================
namespace
{
    constexpr int kSkipLow  = 6;   // four lowest-frequency bins (bottom rows)
    constexpr int kSkipHigh = 2;   // one highest-frequency bin  (top row)

    constexpr int kLines    = 20;  // how many lines you still want to show
}

//------------------------------------------------------------------
void frequencyLines::paint (juce::Graphics& g)
{
    constexpr int kSkipLow  = 3;   // 4 lowest-freq bins skipped
    constexpr int kSkipHigh = 1;   // 1 highest-freq bin skipped
    constexpr int kLines    = 20;  // lines to actually draw

    if ((int) smoothedBins.size() < kSkipLow + kSkipHigh + kLines)
        return;

    const float bassDownScaleIntensity = 1.0f;       // 0 = off, 1 = full slope
    float       bassDownScaleFactor    = 1.0f;

    auto  bounds = getLocalBounds().toFloat();
    float rowH   = bounds.getHeight() / (float) (kLines - 1);
    float width  = bounds.getWidth();

    for (int visIdx = 0; visIdx < kLines; ++visIdx)
    {
        const int binIdx = kSkipLow + visIdx;        // skip four bottom bins
        
        
        bassDownScaleFactor = juce::jmap((float) visIdx, 0.f, (float) (kLines - 1), 0.75f, 0.9f) * bassDownScaleIntensity;
        

        float db      = smoothedBins[binIdx];
        //float clamped = juce::jlimit (-80.f, 0.f, db);
        float norm    = bassDownScaleFactor * bassDownScaleFactor * juce::jmap   (db, -70.f, -10.f, 0.f, 1.f);
        norm          = std::pow (juce::jlimit (0.f, 1.f, 0.9f*norm), 2.5f);

 

        /* ---------------- final line length ----------------------------- */
        float len = 15.f +    juce::jmap (norm, 0.f, 1.f, 0.f, width - 15.f);

        float y   = (kLines - 1 - visIdx) * rowH;

        g.setColour (juce::Colour::fromString ("#FFF7F7F7"));
        g.drawLine  (0.f, y, len, y, 1.f);
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
