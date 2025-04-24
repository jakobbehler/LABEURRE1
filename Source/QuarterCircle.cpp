#include <JuceHeader.h>
#include "QuarterCircle.h"
#include "BinaryData.h"


juce::Image QuarterCircle::framesRed [numFrames];
juce::Image QuarterCircle::framesBlue[numFrames];
bool        QuarterCircle::framesLoaded = false;

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
    loadFramesIfNeeded();
    
    

}


QuarterCircle::~QuarterCircle()
{
}

void QuarterCircle::loadFramesIfNeeded()
{
    if (framesLoaded) return;

    // fill the *class* arrays, not locals
    framesRed [0]  = juce::ImageCache::getFromMemory (BinaryData::grad_R_1_png, BinaryData::grad_R_1_pngSize);
    framesRed [1]  = juce::ImageCache::getFromMemory (BinaryData::grad_R_2_png, BinaryData::grad_R_2_pngSize);
    //framesRed [2]  = juce::ImageCache::getFromMemory (BinaryData::grad_R_3_png, BinaryData::grad_R_3_pngSize);
    //framesRed [3]  = juce::ImageCache::getFromMemory (BinaryData::grad_R_4_png, BinaryData::grad_R_4_pngSize);
    //framesRed [4]  = juce::ImageCache::getFromMemory (BinaryData::grad_R_5_png, BinaryData::grad_R_5_pngSize);

    framesBlue[0]  = juce::ImageCache::getFromMemory (BinaryData::grad_B_1_png, BinaryData::grad_B_1_pngSize);
    framesBlue[1]  = juce::ImageCache::getFromMemory (BinaryData::grad_B_2_png, BinaryData::grad_B_2_pngSize);
    //framesBlue[2]  = juce::ImageCache::getFromMemory (BinaryData::grad_B_3_png, BinaryData::grad_B_3_pngSize);
    //framesBlue[3]  = juce::ImageCache::getFromMemory (BinaryData::grad_B_4_png, BinaryData::grad_B_4_pngSize);
    //framesBlue[4]  = juce::ImageCache::getFromMemory (BinaryData::grad_B_5_png, BinaryData::grad_B_5_pngSize);

    framesLoaded = true;
}



// Add a member:
juce::Path cachedArcPath;

// Update `paint()`:
void QuarterCircle::paint (juce::Graphics& g)
{
    const juce::Image* frames = (rotation == 0 || rotation == 1)
                                ? framesRed
                                : framesBlue;

    float norm   = juce::jmap (radius, smallestRadius, biggestRadius, 0.f, 1.f);
    float scaled = norm * (numFrames - 1);
    int   idxA   = (int) std::floor (scaled);
    int   idxB   = juce::jlimit (0, numFrames - 1, idxA + 1);
    float t      = scaled - (float) idxA;

    g.reduceClipRegion (cachedArcPath);

    g.setOpacity (1.f - t);
    g.drawImage (frames[idxA], getLocalBounds().toFloat());

    g.setOpacity (t);
    g.drawImage (frames[idxB], getLocalBounds().toFloat());

    g.setOpacity (1.f);          // reset
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
    
    juce::Colour blau = juce::Colour::fromString("#FF6184FF");
    juce::Colour rot = juce::Colour::fromString("#FFF0597C");

    
    switch (rotation)
    {
        case 0:
            
            fillColour = rot;
            x = -radius; y = heightF - radius;
            centerPoint = { 0.0f, heightF };
            startArc = 0.0f;
            endArc = juce::MathConstants<float>::halfPi;
            break;
        case 1:
            fillColour = rot;
            x = -radius; y = -radius;
            centerPoint = { 0.0f, 0.0f };
            startArc = juce::MathConstants<float>::halfPi;
            endArc = juce::MathConstants<float>::pi;
            break;
        case 3:
            fillColour = blau;
            x = widthF - radius; y = heightF - radius;
            centerPoint = { widthF, heightF };
            startArc = juce::MathConstants<float>::halfPi;
            endArc = juce::MathConstants<float>::twoPi;
            break;
        case 2:
        default:
            fillColour = blau;
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
    float y = y_position_pixels;
    float x;
    float barwidth = 60.f;
    float topmargin = 40.f;
    float textOffset;

    // This logic assumes y is relative to the editor, not just the component
    float yAdjusted = y + topmargin;

    if (yAdjusted < 174.f)
    {
        x = getWidth() - 65.f;
        textOffset = 0.f;
    }
    else if (yAdjusted < 327.f)
    {
        x = getWidth() - 65.f - (yAdjusted-174.f) * (101.f / 144.f);
        textOffset = 3.f;
    }
    else
    {
        x = getWidth() - 170.5f;
        textOffset = 5.f;
    }
    
    g.setColour(juce::Colour::fromString("#FFF7F7F7"));
    // Red line from curved X start to right edge

    g.drawLine(x, y, getWidth(), y, 2.0f);

    // Draw normal horizontal frequency line
    
    g.drawLine(0, y, x-barwidth, y, 2.0f);

    // Frequency label
    juce::Rectangle<int> textBounds(x-55+textOffset, y - 15, 100, 30);
    g.drawText(juce::String(juce::roundToInt(herz)) + " Hz", textBounds, juce::Justification::left, false);
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
