//
//
// KnobClass.h
//
//


#pragma once
#include <JuceHeader.h>

class myLookAndFeelCope : public juce::LookAndFeel_V4
{
public:
    myLookAndFeelCope();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

private:
    juce::Image img1;

};
