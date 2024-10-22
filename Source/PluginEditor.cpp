/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CopeStortionAudioProcessorEditor::CopeStortionAudioProcessorEditor (CopeStortionAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p)
, driveSliderAttachment(audioProcessor.apvts, "driveAmount", driveSlider)
, rangeSliderAttachment(audioProcessor.apvts, "rangeAmount", rangeSlider)
, drywetSliderAttachment(audioProcessor.apvts, "drywetAmount", drywetSlider)
, volumeSliderAttachment(audioProcessor.apvts, "volumeAmount", volumeSlider)
{
    auto myImage = juce::ImageCache::getFromMemory(BinaryData::BG_png, BinaryData::BG_pngSize);
    if (myImage.isValid())
        logo.setImage(myImage, juce::RectanglePlacement::stretchToFit);
    else
        jassertfalse;
    
    
    driveSlider.setLookAndFeel(&myLookAndFeelCope);
    rangeSlider.setLookAndFeel(&myLookAndFeelCope);
    drywetSlider.setLookAndFeel(&myLookAndFeelCope);
    volumeSlider.setLookAndFeel(&myLookAndFeelCope);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible(logo);
    
    for ( auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    setSize (300, 340);
}

CopeStortionAudioProcessorEditor::~CopeStortionAudioProcessorEditor()
{
}

//==============================================================================
void CopeStortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void CopeStortionAudioProcessorEditor::resized()
{
    
    // lr, ud, lr, ud ( int x, int y, int width, int height)
    logo.setBounds(0, 0, 300, 340);
    driveSlider.setBounds(238,12, 51,51);
    rangeSlider.setBounds(239,102, 51,51);
    drywetSlider.setBounds(239,191, 51,51);
    volumeSlider.setBounds(239,276, 51,51);
}

std::vector<juce::Component*> CopeStortionAudioProcessorEditor::getComps()
{
    return
    {
        &driveSlider,
        &rangeSlider,
        &drywetSlider,
        &volumeSlider
    };
};
