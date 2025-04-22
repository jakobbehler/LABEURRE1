#include <JuceHeader.h>
#include "QuarterCircle.h"

// CIRCLE VARIABLES GLOBAL

const float smallestR = 60.f;
const float biggestR = 170.f;

// Constructor: Initialize radius
QuarterCircle::QuarterCircle(int rotationIndex)
{
    this->rotation = rotationIndex; // member variable stuff in constructor!!!!
    radius = targetRadius = 75.f;
    smallestRadius = smallestR;
    biggestRadius = biggestR;
    
}


QuarterCircle::~QuarterCircle()
{
}


// Add a member:
juce::Path cachedArcPath;

// Update `paint()`:
void QuarterCircle::paint(juce::Graphics& g)
{
    juce::Colour dunkel_farb = juce::Colour::fromString("#FF202426");
    g.setColour(dunkel_farb);
    g.fillPath(cachedArcPath);
}

// Rebuild arc when resized or radius changes:
void QuarterCircle::resized()
{
    rebuildArc();
    repaint();
}

void QuarterCircle::setRadius(float newRadius)
{
    targetRadius = juce::jlimit(smallestRadius, biggestRadius, newRadius);
    startTimerHz(60); // Start interpolating
}

void QuarterCircle::timerCallback()
{
    float smoothing = 0.15f;

    float diff = targetRadius - radius;
    if (std::abs(diff) < 0.1f)
    {
        radius = targetRadius;
        stopTimer(); // Done interpolating
    }
    else
    {
        radius += diff * smoothing;
    }

    rebuildArc();
    repaint();
}


// === New helper ===
void QuarterCircle::rebuildArc()
{
    const float diameter = radius * 2.0f;
    const float widthF = static_cast<float>(getWidth());
    const float heightF = static_cast<float>(getHeight());

    float x = 0.0f, y = 0.0f;
    float startArc = 0.0f, endArc = 0.0f;

    switch (rotation)
    {
        case 0:
            x = -radius; y = heightF - radius;
            centerPoint = { 0.0f, heightF };
            startArc = 0.0f;
            endArc = juce::MathConstants<float>::halfPi;
            break;
        case 1:
            x = -radius; y = -radius;
            centerPoint = { 0.0f, 0.0f };
            startArc = juce::MathConstants<float>::halfPi;
            endArc = juce::MathConstants<float>::pi;
            break;
        case 3:
            x = widthF - radius; y = heightF - radius;
            centerPoint = { widthF, heightF };
            startArc = juce::MathConstants<float>::halfPi;
            endArc = juce::MathConstants<float>::twoPi;
            break;
        case 2:
        default:
            x = widthF - radius; y = -radius;
            centerPoint = { widthF, 0.0f };
            startArc = juce::MathConstants<float>::pi;
            endArc = 3 * juce::MathConstants<float>::halfPi;
            break;
    }

    // Rebuild arc
    cachedArcPath.clear();
    cachedArcPath.startNewSubPath(centerPoint);
    cachedArcPath.addArc(x, y, diameter, diameter, startArc, endArc, true);
    cachedArcPath.lineTo(centerPoint);
    cachedArcPath.closeSubPath();
}





// mouse drag to update the radius
void QuarterCircle::mouseDrag(const juce::MouseEvent& event)
{

    float newRadius = event.position.getDistanceFrom(centerPoint);
    radius = juce::jlimit(smallestR, biggestR, newRadius); // Constrain radius
    
    radius = juce::jlimit(smallestRadius,  biggestRadius , newRadius);
    repaint();
    
    if (onRadiusChanged)
            onRadiusChanged(radius);
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

    // FOR POSITIONING --> SUBTRACT MARGINS
    const int quadW = (w - margin) / 2;
    const int quadH = (h - margin) / 2;

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
    
    g.reduceClipRegion(getLocalBounds());

    // BORDER FOR DEBUGGING!
//    g.setColour(juce::Colours::blue);
//    g.drawRect(getLocalBounds());
//    DBG("[CircleComponent] paint called");
}




// ====================================================================================================================
// FREQ LINE SPLIT
// ===================================================================================================================


const float minY = 0.f;
const float maxY = 350.f;


frequencyLineComponent::frequencyLineComponent()
{
    y_position_pixels = targetY = (maxY-minY)*0.5f;
    isDragging = false;
}

void frequencyLineComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black); //

    // Draw horizontal frequency line
    g.drawLine(0, y_position_pixels, getWidth() - 100, y_position_pixels, 2.0f);

    // Display y-position label
    juce::Rectangle<int> textBounds(getWidth() - 90, y_position_pixels - 15, 100, 30);
    g.drawText(juce::String(juce::roundToInt(herz)) + " Hz", textBounds, juce::Justification::left, false);

    
    //BOUNDS FOR DEBUGGING
//    g.setColour(juce::Colours::red);
//    g.drawRect(getLocalBounds());
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

        //repaint();
        repaint(getLocalBounds());
        if (onYChanged)
            onYChanged(herz);  // Only from user drag
    }
}

    
    void frequencyLineComponent::mouseDown(const juce::MouseEvent& event)
    {
        if (std::abs(event.position.y - y_position_pixels) < 10.0f)
        {
            isDragging = true;
            setInterceptsMouseClicks(true, false);
        }
        else
        {
            setInterceptsMouseClicks(false, false);
        }
    }

    void frequencyLineComponent::mouseUp(const juce::MouseEvent&)
    {
        isDragging = false;
        setInterceptsMouseClicks(true, false);
    }

    bool frequencyLineComponent::hitTest(int x, int y)
    {
        // Only respond if mouse is near the line
        return std::abs(y - y_position_pixels) < 10.0f;
    }


float frequencyLineComponent::getYposition() const
{
    return y_position_pixels;
}


void frequencyLineComponent::setYposition(double y)
{
    targetY = juce::jlimit(minY, maxY, static_cast<float>(y));
    startTimerHz(60); // start interpolation
}


void frequencyLineComponent::updateHerzFromY()
{
    
    const float minHz = 20.0f;
    const float maxHz = 20000.0f;

    float norm = juce::jlimit(0.0f, 1.0f, (y_position_pixels - minY) / (maxY - minY));
    
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
    y_position_pixels = juce::jmap(1.0f - norm, minY, maxY); // flipped BECAUSE Y STARTS AT TOP!!

}


void frequencyLineComponent::timerCallback()
{
    const float smoothing = 0.2f;
    float diff = targetY - y_position_pixels;

    if (std::abs(diff) < 0.3f)
    {
        y_position_pixels = targetY;
        stopTimer();
    }
    else
    {
        y_position_pixels += diff * smoothing;
    }

    updateHerzFromY();
    repaint();

    if (onYChanged)
        onYChanged(herz); // Optional: might want to gate this if not dragging
}
