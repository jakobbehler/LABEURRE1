#include <JuceHeader.h>
#include "QuarterCircle.h"

// CIRCLE VARIABLES GLOBAL

const float smallestR = 60.f;
const float biggestR = 170.f;

// Constructor: Initialize radius
QuarterCircle::QuarterCircle(int rotationIndex)
{
    this->rotation = rotationIndex;  // `this->` clarifies you're assigning to a member variable
    radius = 75.f;
    smallestRadius = smallestR;
    biggestRadius = biggestR;
    
}


QuarterCircle::~QuarterCircle()
{
}


void QuarterCircle::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::black);

    const float diameter = radius * 2.0f;
    const float widthF = static_cast<float>(getWidth());
    const float heightF = static_cast<float>(getHeight());
    
    float startArc = 0.f;
    float endArc = 0.f;
    float x = 0.0f;
    float y = 0.0f;
    juce::Point<float> center;

    // Set position based on rotation
    switch (rotation)
    {
        case 0: // top-right
            
            effectName = "distortionTop";
            x = -radius;
            y = heightF - radius;
            center = juce::Point<float>(0.0f, heightF);
            
            startArc = 0.f;
            endArc = juce::MathConstants<float>::halfPi;
            
            
            break;

        case 1: // bottom-right
            
            effectName = "distortionLow";
            x = -radius;
            y = -radius;
            center = juce::Point<float>(0, 0);
            
            startArc = juce::MathConstants<float>::halfPi;
            endArc = juce::MathConstants<float>::pi;

            
            break;

        case 3:// top-left
            
            effectName = "compressionTop";
                x = widthF - radius;
            y = heightF-radius;
                center = juce::Point<float>(widthF, heightF);

            startArc = juce::MathConstants<float>::halfPi;;
            endArc = juce::MathConstants<float>::twoPi;
            break;

        case 2: // bottom-left
        default:
            
            effectName = "compressionBottom";
            x = widthF - radius;
            y = -radius;
            center = juce::Point<float>(widthF, 0.0f);
            
            startArc = juce::MathConstants<float>::pi;
            endArc = 3 * juce::MathConstants<float>::halfPi;
            break;
    }
    
    centerPoint = center;

    // Create arc path
    juce::Path path;
    path.startNewSubPath(center);
    path.addArc(x, y, diameter, diameter,
                startArc,
                endArc, true);
    path.lineTo(center);
    path.closeSubPath();

    // Draw arc
    g.fillPath(path);

    // Label and hover styling
    g.setColour(isHovered ? juce::Colours::white : juce::Colours::black);
    juce::String text =juce::String(effectName) + " Radius: " + juce::String(radius);
    g.drawText(text,
               getLocalBounds(),
               juce::Justification::topRight);
    
    g.setColour(juce::Colours::green);
    g.drawRect(getLocalBounds());
}





// Handle mouse drag to update the radius
void QuarterCircle::mouseDrag(const juce::MouseEvent& event)
{
    //juce::Point<float> center(getWidth(), 0.f);
    //juce::Point<float> center(getWidth(), 0.f);
    

    float newRadius = event.position.getDistanceFrom(centerPoint);
    radius = juce::jlimit(smallestR, biggestR, newRadius); // Constrain radius
    

    radius = juce::jlimit(smallestRadius,  biggestRadius , newRadius);
    repaint();
    
    if (onRadiusChanged)
            onRadiusChanged(radius);
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

void QuarterCircle::setRadius(float newRadius)
{
    radius = juce::jlimit(smallestRadius, biggestRadius, newRadius);
    repaint();
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
    const int margin = 10;
    const auto w = getWidth();
    const auto h = getHeight();

    // Total size to subtract from full width/height due to margin in between
    const int quadW = (w - margin) / 2;
    const int quadH = (h - margin) / 2;

    // Position the 4 quarter circles with a 20px margin in between
    quads[0].setBounds(quadW + margin, 0, quadW, quadH);                // top-right
    quads[1].setBounds(quadW + margin, quadH + margin, quadW, quadH);   // bottom-right
    quads[2].setBounds(0, quadH + margin, quadW, quadH);                // bottom-left
    quads[3].setBounds(0, 0, quadW, quadH);                             // top-left
}


QuarterCircle& CircleComponent::getQuad(int index)
{
    jassert(index >= 0 && index < 4);  // Safety check
    return quads[index];
}

void CircleComponent::paint(juce::Graphics& g)
{
    
    g.reduceClipRegion(getLocalBounds()); // ✅ Clip to bounds of CircleComponent

    // Optional debug border:
    g.setColour(juce::Colours::blue);
    g.drawRect(getLocalBounds());
    DBG("[CircleComponent] paint called"); // ✅ check terminal
}


// ====================================================================================================================
// FREQ LINE SPLIT
// ===================================================================================================================


const float minY = 0.f;
const float maxY = 350.f;


frequencyLineComponent::frequencyLineComponent()
{
    y_position_pixels = (maxY-minY)*0.5f ;
    isDragging = false;
}

void frequencyLineComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black); //

    // Draw horizontal frequency line
    g.drawLine(0, y_position_pixels, getWidth() - 100, y_position_pixels, 2.0f);

    // Display y-position label
    juce::Rectangle<int> textBounds(getWidth() - 90, y_position_pixels - 15, 100, 30);
    g.drawText(juce::String(herz) + " Hz", textBounds, juce::Justification::left, false);
    
    g.setColour(juce::Colours::red);
    g.drawRect(getLocalBounds());
}

void frequencyLineComponent::resized()
{
    // Ensure y_position stays inside bounds
    y_position_pixels = juce::jlimit(minY, maxY, y_position_pixels);
    
    if (y_position_pixels == 0.0f){
        y_position_pixels = getHeight() / 2.0f;
    }
}
  
void frequencyLineComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (isDragging)
    {
        y_position_pixels = juce::jlimit(minY, maxY, event.position.y);
     

        updateHerzFromY();  // Updates `herz`

        repaint();

        if (onYChanged)
            onYChanged(herz);  // Only from user drag
    }
}

    
    void frequencyLineComponent::mouseDown(const juce::MouseEvent& event)
    {
        if (std::abs(event.position.y - y_position_pixels) < 10.0f)
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
        return std::abs(y - y_position_pixels) < 10.0f;
    }


//void frequencyLineComponent::mouseUp(const juce::MouseEvent&)
//{
//    isDragging = false;
//}




float frequencyLineComponent::getYposition() const
{
    return y_position_pixels;
}


void frequencyLineComponent::setYposition(double y)
{
    y_position_pixels = static_cast<float>(y);
}

void frequencyLineComponent::updateHerzFromY()
{
    // You can make this log scale if needed
    
    const float minHz = 20.0f;
    const float maxHz = 20000.0f;

    float norm = juce::jlimit(0.0f, 1.0f, (y_position_pixels - minY) / (maxY - minY));
    
    //float logHz = juce::jmap(norm, std::log10(minHz), std::log10(maxHz)); // old
    
    float logHz = juce::jmap(1.0f - norm, std::log10(minHz), std::log10(maxHz)); // flipped

    herz = std::pow(10.0f, logHz);
}

float frequencyLineComponent::getHerz() const
{
    return herz;
}


void frequencyLineComponent::setHerz(float newHerz)
{
    herz = juce::jlimit(20.0f, 20000.0f, newHerz); // Sanity check
    updateYFromHerz();
    repaint();
}

void frequencyLineComponent::updateYFromHerz()
{

    const float minHz = 20.0f;
    const float maxHz = 20000.0f;

    float logHz = std::log10(herz);
    float norm = (logHz - std::log10(minHz)) / (std::log10(maxHz) - std::log10(minHz));
    //y_position_pixels = juce::jmap(norm, minY, maxY); //old
    y_position_pixels = juce::jmap(1.0f - norm, minY, maxY); // flipped

}


