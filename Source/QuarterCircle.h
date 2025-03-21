#pragma once

#include <JuceHeader.h>







class QuarterCircle : public juce::Component
{
public:
    QuarterCircle(int rotationIndex);
    ~QuarterCircle() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDrag(const juce::MouseEvent& event) override; // Allow radius change
    float getRadius() const;
    
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    
private:
    float radius; // Store the radius of the quarter-circle
    float smallestRadius;
    float biggestRadius;
    int rotation;
    juce::Point<float> centerPoint;
    
    
    
    
    bool isHovered;
    
    juce::Image gradientImage;  // Store the gradient image

    void generateGradient();  // Step 1: Create gradient
    void drawGrain(juce::Graphics& g);  // Step 3: Generate grain effect
    float halton(int index, int base);  // Generate quasi-random points
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuarterCircle)
};


class CircleComponent : public juce::Component
{
public:
    CircleComponent();
    ~CircleComponent() override = default;

    void resized() override;

private:
    
    float y_position;


    
    QuarterCircle quads[4] = {
        QuarterCircle(0),
        QuarterCircle(1),
        QuarterCircle(2),
        QuarterCircle(3)
    };
};



class frequencyLineComponent : public juce::Component
{
    public:
        frequencyLineComponent();
        ~frequencyLineComponent() override = default;
    
        

    void paint(juce::Graphics&) override;
    
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    bool hitTest(int x, int y) override;
    
    float getYposition() const;
    
    std::function<void(float)> onYChanged;  // 👈 signal to parent


    private:
        
    float y_position;
    bool isDragging;
        
    
};
