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

// Paint method: Draws a quarter-circle
void QuarterCircle::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::green);
    g.setColour(juce::Colours::white);


    juce::Point<float> center(getHeight(), 0.f);//);

        juce::Path path;
    
        path.startNewSubPath(center);
    
        path.addArc(0, getHeight() - (radius * 2),  // X, Y position
                    radius * 2, radius * 2,         // Width, Height of the bounding box
                    juce::MathConstants<float>::pi,
                    juce::MathConstants<float>::halfPi * 3, true);
 
        
        path.lineTo(center);
        
    
        g.fillPath(path);
}

// Handle mouse drag to update the radius
void QuarterCircle::mouseDrag(const juce::MouseEvent& event)
{
    juce::Point<float> center(getWidth() / 2.0f, getHeight());

    float newRadius = event.position.getDistanceFrom(center);
    radius = juce::jlimit(10.0f, 300.0f, newRadius); // Constrain radius
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
