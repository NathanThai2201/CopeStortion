/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>

//==============================================================================
CopeStortionAudioProcessor::CopeStortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

CopeStortionAudioProcessor::~CopeStortionAudioProcessor()
{
}

//==============================================================================
const juce::String CopeStortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CopeStortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CopeStortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CopeStortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CopeStortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CopeStortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CopeStortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CopeStortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CopeStortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void CopeStortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CopeStortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CopeStortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CopeStortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CopeStortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    auto chainSettings = getChainSettings(apvts);
    float drive = chainSettings.drive;
    float range = chainSettings.range;
    float drywet = chainSettings.drywet;
    float volume = chainSettings.volume;
    
    // sin(x) parameters
    float frequency = 440.0f;
    float sineAmplitude = 0.1f;
    float phase = 0.0f;
    float sampleRate = getSampleRate();
    static float phaseAccumulator = 0.1f;
    
    /**
     
        MAIN LOOP <EACH CHANNEL>
     
     **/
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        float previousSample = 0.0f;
        
        /**
         
            SMALL LOOP <EACH SAMPLE>
         
         **/
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            float cleanSig = *channelData;
            
            *channelData *= drive * range;

            
            // Copestortion algorithm:
            // A. (2/pi) * arctan(a) + cos(a)/2     +     (a-1) * 0.3 * sin(10x)
            // B. Sine fold 2
            // C. + 0.2 * previous a
            // D. hardclip at 1.5
            
            //A.
            *channelData = ((2.f/M_PI) * atan(*channelData)+cos(*channelData)/2);
            
            // sin(10x)
            float sineValue = sineAmplitude * std::sin(2.0f * M_PI * 10.0f * frequency * phaseAccumulator / sampleRate + phase);

            // Add the sine wave to the current audio sample (not at 0 to counteract buzzing)
            if (volume!=0)
            {
                *channelData += (*channelData-1) * 0.3 * sineValue;
            }
            // sine wave update phase accumulator
            phaseAccumulator += 1.0f;
            if (phaseAccumulator >= sampleRate)
                phaseAccumulator -= sampleRate;
            
            // sine fold 2
            //B.
            *channelData = sin(*channelData*2);
            
            //C.
            previousSample = *channelData;
            *channelData += 0.2 * previousSample;
            
            
            //D.
            if (*channelData>1.5){
                *channelData=1.5;
            }
            
            //drywet control
            *channelData = ((*channelData * drywet) + (cleanSig * (1.f-drywet)))*volume;
            channelData++;
        }
        // ..do something to the data...
    }
}

//==============================================================================
bool CopeStortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CopeStortionAudioProcessor::createEditor()
{
    return new CopeStortionAudioProcessorEditor (*this);
    //EDITOR VIEW COMMENT
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void CopeStortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CopeStortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.drive = apvts.getRawParameterValue("driveAmount")->load();
    settings.range = apvts.getRawParameterValue("rangeAmount")->load();
    settings.drywet = apvts.getRawParameterValue("drywetAmount")->load();
    settings.volume = apvts.getRawParameterValue("volumeAmount")->load();
    
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout CopeStortionAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("driveAmount",1),"driveAmount",juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f, 1.0f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("rangeAmount",1),"rangeAmount",juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f, 1.0f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("drywetAmount",1),"drywetAmount",juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f, 1.0f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("volumeAmount",1),"volumeAmount",juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 0.5f));
    
    
    return layout;
};



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CopeStortionAudioProcessor();
}
