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
    
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    
private:
    float radius; // Store the radius of the quarter-circle
    bool isHovered;
    
    juce::Image gradientImage;  // Store the gradient image

    void generateGradient();  // Step 1: Create gradient
    void drawGrain(juce::Graphics& g);  // Step 3: Generate grain effect
    float halton(int index, int base);  // Generate quasi-random points
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuarterCircle)
};
