/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

//==============================================================================
/**
*/
class VSTEmotionRendererAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    VSTEmotionRendererAudioProcessorEditor (VSTEmotionRendererAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~VSTEmotionRendererAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VSTEmotionRendererAudioProcessor& audioProcessor;

    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::Label mixLabel;
    juce::Slider mixSlider;
    std::unique_ptr<SliderAttachment> mixAttachment;

    VSTEmotionRendererAudioProcessor* getProcessor() const
    {
        return static_cast <VSTEmotionRendererAudioProcessor*> (getAudioProcessor());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VSTEmotionRendererAudioProcessorEditor)
};
