/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParamHelpers.h"

//==============================================================================
Looper_testAudioProcessor::Looper_testAudioProcessor()
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

Looper_testAudioProcessor::~Looper_testAudioProcessor()
{
}

//==============================================================================
const juce::String Looper_testAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Looper_testAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Looper_testAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Looper_testAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Looper_testAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Looper_testAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Looper_testAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Looper_testAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Looper_testAudioProcessor::getProgramName (int index)
{
    return {};
}

void Looper_testAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Looper_testAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    loop_buf_.resize (sampleRate * 8, 0.f);
    
    looper_.Init (loop_buf_.data(), loop_buf_.size());
    looper_.state_ = Looper::LISTENING;
}

void Looper_testAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Looper_testAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Looper_testAudioProcessor::updateParams()
{
    auto get_param_val = [&](juce::String name) -> float {
        return apvts.getRawParameterValue (name)->load();
    };
    
    params_.trig_ = get_param_val ("trig");
    params_.division_ = get_param_val ("division");
    params_.seg_sel_ = get_param_val ("seg-sel");
    params_.time_manip_ = get_param_val ("time-manip");
}

void Looper_testAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    bool current_toggle_state = apvts.getRawParameterValue("trig")->load();
    
    if (current_toggle_state != previous_toggle_state_)
    {
        looper_.UpdatePlaybackState();
    }
    
    previous_toggle_state_ = current_toggle_state;
    
    looper_.SetSegmentDivisions ( apvts.getRawParameterValue( "division")   ->load());
    looper_.SetSelectedSegment  ( apvts.getRawParameterValue( "seg-sel")    ->load());
    looper_.SetTimeManipulation ( apvts.getRawParameterValue( "time-manip") ->load());
    
    for (int s = 0; s < buffer.getNumSamples(); s++)
    {
        auto in_s = buffer.getSample (0, s);
        
        auto out_s = looper_.Process (in_s);
        
        buffer.setSample (0, s, out_s); // l
        buffer.setSample (1, s, out_s); // r
    }
}

//==============================================================================
bool Looper_testAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Looper_testAudioProcessor::createEditor()
{
//    return new Looper_testAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void Looper_testAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Looper_testAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout Looper_testAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    
    using namespace param_helpers;
    
    layout.add (ap_bool (id ("trig"),
                         false));

    layout.add (ap_float (id ("division"),
                          {0.f, 1.f, 0.00001f, 1.f},
                          0));

    layout.add (ap_float (id ("seg-sel"),
                          {0.f, 1.f, 0.00001f, 1.f},
                          0));

    layout.add (ap_float (id ("time-manip"),
                          {0.f, 1.f, 0.00001f, 1.f},
                          0));
    
    
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Looper_testAudioProcessor();
}
