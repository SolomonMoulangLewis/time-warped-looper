/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "looper.h"
#include "dsp.h"

//==============================================================================
/**
*/
class Looper_testAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Looper_testAudioProcessor();
    ~Looper_testAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS::ParameterLayout createParameterLayout();
    APVTS apvts {*this, nullptr, "Parameter Layout", createParameterLayout()};
    enum Names {
        TRIG,
        DIVISION,
        SEG_SEL,
        TIME_MANIP
    };
    std::map<Names, juce::String> param_names_ {
        {TRIG, "trig"},
        {DIVISION, "division"},
        {SEG_SEL, "seg-sel"},
        {TIME_MANIP, "time-manip"}
    };
    struct {
        bool trig_;
        float division_;
        float seg_sel_;
        float time_manip_;
    } params_;
    void updateParams();
    
    bool loop_toggle_ = false;
    bool previous_toggle_state_ = false;
    
    Looper looper_;
    std::vector<float> loop_buf_;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Looper_testAudioProcessor)
};
