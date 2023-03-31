/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VSTEmotionRendererAudioProcessor::VSTEmotionRendererAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, juce::Identifier("VSTEmotionRendererAPVTS"), createParameterLayout())
#endif
{

    mixParameter = parameters.getRawParameterValue("mix");//mixParameter is a pointer
    filterBuffer = juce::AudioSampleBuffer(1,1);
    tempBuffer = juce::AudioSampleBuffer(1,1);
    myWaveTable = WaveTable::AllWaveTable();
    
}

VSTEmotionRendererAudioProcessor::~VSTEmotionRendererAudioProcessor()
{
}

//==============================================================================
const juce::String VSTEmotionRendererAudioProcessor::getName() const
{
    return JucePlugin_Name;
}


bool VSTEmotionRendererAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VSTEmotionRendererAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VSTEmotionRendererAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VSTEmotionRendererAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VSTEmotionRendererAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VSTEmotionRendererAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VSTEmotionRendererAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VSTEmotionRendererAudioProcessor::getProgramName (int index)
{
    return {};
}

void VSTEmotionRendererAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VSTEmotionRendererAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    const auto numChannels = getNumInputChannels();
    for (size_t ch = 0; ch < numChannels; ch++)
    {
        filterMatrix.emplace_back(FilterArray());
        for (size_t i = 0; i < 48; i++)
        {
            filterMatrix[ch][i].setCoefficients(sampleRate, freqList[i], 5.0f);
        }
    }
    tempBuffer.setSize(1, samplesPerBlock);
    tempBuffer.clear();
    filterBuffer.setSize(1, samplesPerBlock);
    filterBuffer.clear();
}

void VSTEmotionRendererAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VSTEmotionRendererAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VSTEmotionRendererAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto *data = buffer.getWritePointer (channel);
        auto numSamples = buffer.getNumSamples();
        // Pass to tone filter array.
        
        auto *outData = filterBuffer.getWritePointer(0);
        
        for (auto &&filter : filterMatrix[channel])
        {
            auto *tempData = tempBuffer.getWritePointer(0);

            filter.process(data, tempData, numSamples);

            // AM modulation
            //auto rms = filter.calculateRMS(tempData, numSamples);
            //
            for (size_t i = 0; i < numSamples; ++i)
            {
                waveTableIndex = (waveTableIndex+1) % myWaveTable.tableTriangle.size();
                tempData[i] = tempData[i] * myWaveTable.tableTriangle[waveTableIndex];//do AM
            }
            //

            for (size_t i = 0; i < numSamples; ++i)
            {
                outData[i] += tempData[i];
            }
            /*
            for (size_t i = 0; i < numSamples; ++i)
            {
                waveTableIndex = (waveTableIndex+1) % myWaveTable.tableAlchemyPeak.size();
                outData[i] = data[i] * myWaveTable.tableAlchemyPeak[waveTableIndex];//do AM
            }
            */


            tempBuffer.clear(0, 0, tempBuffer.getNumSamples());
        }

        for (size_t i = 0; i < numSamples; ++i)
        {
            auto mix = mixParameter->load();
            data[i] = outData[i] * mix + data[i] * (1 - mix);
            //data[i] = outData[i];
        }
        filterBuffer.clear(0, 0, filterBuffer.getNumSamples());
    }


}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout VSTEmotionRendererAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;
    params.add(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));//min max minimal_change default

    return params;
}
//==============================================================================
bool VSTEmotionRendererAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VSTEmotionRendererAudioProcessor::createEditor()
{
    return new VSTEmotionRendererAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void VSTEmotionRendererAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void VSTEmotionRendererAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if(xmlState.get() != nullptr)
        if(xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VSTEmotionRendererAudioProcessor();
}
