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

    g.fillPath(path);
}



// Handle mouse drag to update the radius
void QuarterCircle::mouseDrag(const juce::MouseEvent& event)
{
    juce::Point<float> center(getWidth() / 2.0f, getHeight());
    //juce::Point<float> center(getWidth(), 0.f);
    
    float newRadius = event.position.getDistanceFrom(center);
    radius = juce::jlimit(50.0f, 300.0f, newRadius); // Constrain radius
    repaint();
}



// Handle resizing
void QuarterCircle::resized()
{
    repaint(); // Redraw when resized
}

// Getter for radius
float QuarterCircle::getRadius() const
{
    return radius;
}
