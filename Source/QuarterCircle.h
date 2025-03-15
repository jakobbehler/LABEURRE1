#pragma once

#include <JuceHeader.h>

class QuarterCircle : public juce::Component
{
public:
    QuarterCircle();
    ~QuarterCircle() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDrag(const juce::MouseEvent& event) override; // Allow radius change
    float getRadius() const;
    
private:
    float radius; // Store the radius of the quarter-circle
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuarterCircle)
};
