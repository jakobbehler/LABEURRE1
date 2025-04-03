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
    void setRadius(float newRadius);
    
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    std::function<void(float)> onRadiusChanged;

    
private:
    float radius; // Store the radius of the quarter-circle
    float smallestRadius;
    float biggestRadius;
    int rotation;
    juce::String effectName;
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
    QuarterCircle& getQuad(int index);
    void paint(juce::Graphics&) override;



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

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent&) override;
    bool hitTest(int x, int y) override;

    float getYposition() const;
    void setYposition(double y);

    float getHerz() const;
    void setHerz(float newHerz);
    void updateYFromHerz();

    std::function<void(float)> onYChanged;

private:
    float y_position_pixels = 400.0f;
    float herz = 1000.0f;

    bool isDragging = false;

    void updateHerzFromY();
};

