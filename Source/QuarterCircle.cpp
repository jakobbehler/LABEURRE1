#include <JuceHeader.h>
#include "QuarterCircle.h"



// Constructor: Initialize radius
QuarterCircle::QuarterCircle(int rotationIndex)
{
    this->rotation = rotationIndex;  // `this->` clarifies you're assigning to a member variable
    radius = 75.f;
    smallestRadius = 50.f;
    biggestRadius = 200.f;
    
}


QuarterCircle::~QuarterCircle()
{
}

void QuarterCircle::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);  // Background color
    g.setColour(juce::Colours::black);  // Circle color

    float diameter = radius * 2.0f;

    // Adjust bounding rectangle to keep center at (getWidth(), 0)
    float x = getWidth() - radius;  // Center horizontally stays at getWidth()
    float y = -radius;              // Center vertically stays at 0

    juce::Point<float> center(getWidth(), 0.0f);

    juce::Path path;
    path.startNewSubPath(center);  // Starting at the circle's center

    // Add a quarter-circle arc (bottom-left quarter)
    path.addArc(x, y, diameter, diameter,
                juce::MathConstants<float>::pi,
                3 * juce::MathConstants<float>::halfPi, true);

    path.lineTo(center);  // Close the wedge by connecting back to the center
    path.closeSubPath();
    

    g.fillPath(path);  // Normal rendering without blur
    
    g.drawImageAt(gradientImage, 0, 0);
    drawGrain(g);
    g.setColour(isHovered ? juce::Colours::white : juce::Colours::black);
    
    g.drawText("Radius: " + juce::String(radius), getLocalBounds(),
               juce::Justification::topRight);
    
    
}



// Handle mouse drag to update the radius
void QuarterCircle::mouseDrag(const juce::MouseEvent& event)
{
    juce::Point<float> center(getWidth(), 0.f);
    //juce::Point<float> center(getWidth(), 0.f);
    

    float newRadius = event.position.getDistanceFrom(center);
    radius = juce::jlimit(100.0f, 200.0f, newRadius); // Constrain radius
    

    radius = juce::jlimit(smallestRadius,  biggestRadius , newRadius);
    repaint();
}



// Handle resizing
void QuarterCircle::resized()
{
    repaint(); // Redraw when resized
    //generateGradient();
}

// Getter for radius
float QuarterCircle::getRadius() const
{
    return radius;
}


void QuarterCircle::mouseEnter(const juce::MouseEvent& event)
    {
        isHovered = true;
        repaint();
    }

    void QuarterCircle::mouseExit(const juce::MouseEvent& event)
    {
        isHovered = false;
        repaint();
    }


void QuarterCircle::generateGradient()
{
    int w = getWidth();
    int h = getHeight();

    gradientImage = juce::Image(juce::Image::RGB, w, h, true);
    juce::Graphics g(gradientImage);

    juce::ColourGradient gradient(juce::Colours::black, 0, 0,
                                  juce::Colours::white, w, h, false);
    g.setGradientFill(gradient);
    g.fillAll();
}

float QuarterCircle::halton(int index, int base)
{
    float result = 0;
    float f = 1.0f / base;
    int i = index;

    while (i > 0)
    {
        result += f * (i % base);
        i = std::floor(i / base);
        f /= base;
    }

    return result;
}

void QuarterCircle::drawGrain(juce::Graphics& g)
{
    int w = gradientImage.getWidth();
    int h = gradientImage.getHeight();
    
    float dotSize = 4.0f;  // Max dot size
    float density = 1.f;  // Higher density = more grains
    float threshold = 0.15f;  // Controls visibility

    int area = std::round((w * density) * (h * density));

    // **Clip the drawing region to the quarter-circle**
    juce::Path clipPath;
    float diameter = radius * 2.0f;
    float x = getWidth() - radius;
    float y = -radius;
    juce::Point<float> center(getWidth(), 0.0f);
    
    clipPath.startNewSubPath(center);
    clipPath.addArc(x, y, diameter, diameter,
                    juce::MathConstants<float>::pi,
                    3 * juce::MathConstants<float>::halfPi, true);
    clipPath.lineTo(center);
    clipPath.closeSubPath();

    g.reduceClipRegion(clipPath);  // **Restrict drawing area to the quarter-circle**

    for (int i = 0; i < area; i++)
    {
        float px = halton(i, 2) * w;
        float py = halton(i, 3) * h;

        // Get brightness from the gradient
        juce::Colour pixelColor = gradientImage.getPixelAt((int)px, (int)py);
        float brightness = pixelColor.getBrightness();

        brightness = juce::jmap(brightness, 0.0f, 1.0f, 0.0f, 1.0f - threshold);
        float pointSize = dotSize * (1.0f - brightness);

        if (juce::Random::getSystemRandom().nextFloat() >= brightness)
        {
            g.setColour(juce::Colours::black);
            g.fillEllipse(px, py, pointSize, pointSize);
        }
    }

}


// ====================================================================================================================
// PARENT
// ===================================================================================================================


CircleComponent::CircleComponent()
{
    for (auto& q : quads)
        addAndMakeVisible(q);
}

void CircleComponent::resized()
{
    auto w = getWidth() / 2;
    auto h = getHeight() / 2;

    quads[0].setBounds(w, 0, w, h);   // top-right
    quads[1].setBounds(w, h, w, h);   // bottom-right
    quads[2].setBounds(0, h, w, h);   // bottom-left
    quads[3].setBounds(0, 0, w, h);   // top-left
}

// ====================================================================================================================
// PARENT
// ===================================================================================================================



frequencyLineComponent::frequencyLineComponent()
{
    y_position = 400.f;
    isDragging = false; 
}

void frequencyLineComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);

    // Draw horizontal frequency line
    g.drawLine(0, y_position, getWidth()-100, y_position, 2.0f);

    // Display y-position
    juce::Rectangle<int> textBounds(getWidth()-90, y_position -15, 100, 30);
    g.drawText(juce::String(getHeight() - y_position) + " Hz", textBounds, juce::Justification::left, false);
}

void frequencyLineComponent::resized()
{
    // Ensure y_position stays inside bounds
    y_position = juce::jlimit(0.0f, (float)getHeight(), y_position);
    
    if (y_position == 0.0f){
        y_position = getHeight() / 2.0f;
    }
}
  



//void frequencyLineComponent::mouseDown(const juce::MouseEvent& event)
//    {
//    float dragThreshold = 10.0f;
//    // Start dragging only if the mouse is close to the line
//    if (std::abs(event.position.y - y_position) < dragThreshold)
//        isDragging = true;
//}

void frequencyLineComponent::mouseDrag(const juce::MouseEvent& event)
    {
    if (isDragging)
    {
        y_position = juce::jlimit(0.0f, (float)getHeight(), event.position.y);
        repaint();
        if (onYChanged){
            onYChanged(y_position);  // 👈 notify parent live
        }
    }
    

}

    
    void frequencyLineComponent::mouseDown(const juce::MouseEvent& event)
    {
        if (std::abs(event.position.y - y_position) < 10.0f)
        {
            isDragging = true;
            setInterceptsMouseClicks(true, false);  // ✅ Capture clicks when near the line
        }
        else
        {
            setInterceptsMouseClicks(false, false); // ✅ Let clicks pass through to CircleComponent
        }
    }

    void frequencyLineComponent::mouseUp(const juce::MouseEvent&)
    {
        isDragging = false;
        setInterceptsMouseClicks(true, false);  // ✅ Restore normal behavior
    }

    bool frequencyLineComponent::hitTest(int x, int y)
    {
        // Only respond if mouse is near the line
        return std::abs(y - y_position) < 10.0f;
    }


//void frequencyLineComponent::mouseUp(const juce::MouseEvent&)
//{
//    isDragging = false;
//}


float frequencyLineComponent::getYposition() const
{
    return y_position;
}



