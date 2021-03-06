#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class MpasAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    MpasAudioProcessor();
    ~MpasAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
	void pushNextSampleIntoFifo(float sample) noexcept;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
	// Constants for FFT Stuff
	enum
	{
		fftOrder = 10, // size of the FFT window - 2 ^ fftOrder
		fftSize = 1 << fftOrder // Left bit shift operator to produce 2*fftOrder as a binary number
	};

	bool nextFFTBlockReady = false; // This temporary boolean tells us whether the next FFT block is ready to be rendered.
	float fftData[2 * fftSize]; // The fftData float array of size 2048 will contain the results of our FFT calculations.
	float bufferData[fftSize]; // This array contains buffer data (for ACF)
	int numZeroCrossing; // Number of times a signal crosses zero in a buffer frame

private:
	//==============================================================================
	// FFT Variables
	
	float fifo[fftSize]; // The fifo float array of size 1024 will contain our incoming audio data in samples.
	int fifoIndex = 0; // This temporary index keeps count of the amount of samples in the fifo.
	float prevSample = 0; // For determining if we've crossed zero
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MpasAudioProcessor)
};
