#include <JuceHeader.h>
#include "QuarterCircle.h"

// Constructor: Initialize radius
QuarterCircle::QuarterCircle()
{
    radius = 100.0f;  // Default radius
   
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
    radius = juce::jlimit(100.0f, 300.0f, newRadius); // Constrain radius
    repaint();
}



// Handle resizing
void QuarterCircle::resized()
{
    repaint(); // Redraw when resized
    generateGradient();
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

