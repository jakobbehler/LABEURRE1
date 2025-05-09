#pragma once

#include <JuceHeader.h>

class SimpleEQAudioProcessorEditor; // forward declare!

class QuarterCircle : public juce::Component, private juce::Timer
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
    void rebuildArc();
    void timerCallback() override;
    
    static constexpr int numFrames = 5;
    
    
    void mouseDown(const juce::MouseEvent&) override { setMouseCursor(clickCursor); }
    void mouseUp(const juce::MouseEvent&) override   { setMouseCursor(normalCursor); }

    

    
private:
    
    
    float radius; // Store the radius of the quarter-circle
    
    float targetRadius;
    float smallestRadius;
    float biggestRadius;
    int rotation;
    juce::String effectName;
    juce::Point<float> centerPoint;
    juce::Path cachedArcPath;
    
    juce::Colour fillColour = juce::Colour::fromString("#FFF7F7F7");
    juce::FillType fill;
    
    static juce::Image framesRed[numFrames];
    static juce::Image framesBlue[numFrames];
    static bool        framesLoaded;          // guard (idk the GUI thread has issues with timers???)
    static void        loadFramesIfNeeded();  // helper
    
    bool isHovered;
    bool isDragging = false;
    
    juce::MouseCursor normalCursor;
    juce::MouseCursor clickCursor;
    
    class SimpleEQAudioProcessorEditor; // forward declaration
    SimpleEQAudioProcessorEditor* editorRef = nullptr;
    
   // juce::Image gradientImage;  // Store the gradient image

    //void generateGradient();  // Step 1: Create gradient
    //void drawGrain(juce::Graphics& g);  // Step 3: Generate grain effect
    //float halton(int index, int base);  // Generate quasi-random points
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
    
    void mouseDown(const juce::MouseEvent&) override { setMouseCursor(clickCursor); }
    void mouseUp(const juce::MouseEvent&) override   { setMouseCursor(normalCursor); }

    


private:
    
    float y_position;
    
    juce::MouseCursor normalCursor;
    juce::MouseCursor clickCursor;
    
    
    QuarterCircle quads[4] = {
        QuarterCircle(0),
        QuarterCircle(1),
        QuarterCircle(2),
        QuarterCircle(3)
    };
};



class frequencyLineComponent : public juce::Component, private juce::Timer
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
    
    void timerCallback() override;
    
    
    
    void setTargetHerz(float newTargetHerz);




private:
    
    juce::MouseCursor normalCursor;
    juce::MouseCursor clickCursor;
    
    float y_position_pixels;
    float targetY;
    float herz = 1000.0f;
    float targetHerz = 1000.0f;

    bool isDragging = false;

    void updateHerzFromY();
};

