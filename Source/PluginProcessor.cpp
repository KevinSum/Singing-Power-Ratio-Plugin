#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MpasAudioProcessor::MpasAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )

#endif
{
}

MpasAudioProcessor::~MpasAudioProcessor()
{
}

//==============================================================================
const String MpasAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MpasAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MpasAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MpasAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MpasAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MpasAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MpasAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MpasAudioProcessor::setCurrentProgram (int index)
{
}

const String MpasAudioProcessor::getProgramName (int index)
{
    return {};
}

void MpasAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MpasAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MpasAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MpasAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void MpasAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
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
    //for (int channel = 0; channel < totalNumInputChannels; ++channel)
    for (int channel = 0; channel < 1; ++channel)
	{
        auto* channelData = buffer.getWritePointer (channel);	

		for (auto i = 0; i < buffer.getNumSamples(); ++i)
		{
			pushNextSampleIntoFifo(channelData[i]);
		}

    }

}

void MpasAudioProcessor::pushNextSampleIntoFifo (float sample) noexcept
{
	// if the fifo contains enough data, set a flag to say
	// that the next FFT block should be processed
    if (fifoIndex == fftSize)   // [9] Every time this function gets called, a sample is stored in the fifo and the index is incremented.
    {
        if (!nextFFTBlockReady) 
        {
            zeromem (fftData, sizeof (fftData));
            memcpy (fftData, fifo, sizeof (fifo));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
		numZeroCrossing = 0;
    }

	// If zero crossed, increment (only for that fft block)
	if ((prevSample < 0 && sample >= 0) || (prevSample >= 0 && sample < 0) && !nextFFTBlockReady)
		numZeroCrossing++;

	prevSample = sample; // Update previous sample
    fifo[fifoIndex++] = sample;  // [9] Every time this function gets called, a sample is stored in the fifo and the index is incremented.
}

//==============================================================================
bool MpasAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MpasAudioProcessor::createEditor()
{
    return new MpasAudioProcessorEditor (*this);
}

//==============================================================================
void MpasAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MpasAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MpasAudioProcessor();
}
