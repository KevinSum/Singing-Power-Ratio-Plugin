#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MpasAudioProcessorEditor  : public AudioProcessorEditor,
								  public Timer
{
public:
    MpasAudioProcessorEditor (MpasAudioProcessor&);
    ~MpasAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
	void timerCallback() override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MpasAudioProcessor& processor;

	enum
	{
		fftOrder = 10, // size of the FFT window - 2 ^ fftOrder
		fftSize = 1 << fftOrder // Left bit shift operator to produce 2*fftOrder as a binary number
	};


	SmoothedValue<float, ValueSmoothingTypes::Multiplicative> smoothedValue; // Declare smoothing class


	// Declare a dsp::FFT object to perform the forward FFT on.
	dsp::FFT forwardFFT;  // FFT constructor
	dsp::WindowingFunction<float> window; // Window for FFT

	// Arrays to store data
	float fftData[2 * fftSize];
	float prevFFTData[2 * fftSize] = {}; // To use to calculate spectral flux
	float bufferData[fftSize]; // For auto-correlation
	float prevBufferData[fftSize]; // Delay signal for auto-correlation

	double maxACF; // Auto Correlation Function value

	float spectralFlux;
	double normalizationMultiplier;

	// Singers formant stuff
	float lowFreqsLim = 2000; // Threshold between first two formants and the Singers Formant (Hz)
	float highFreqsLim = 4000; // Upper limit
	int lowFreqsLimIdx = (lowFreqsLim * fftSize) / processor.getSampleRate(); // FFT Index for threshold freq
	int highFreqsLimIdx = (highFreqsLim * fftSize) / processor.getSampleRate(); // FFT Index for freq limit
	float highPartial;
	float lowPartial;

	float lowFreqsPowDensity; // Power Density of lower freq band
	float highFreqsPowDensity; // Power Density of high freq band

	float minFormantRatio = 45; // Min value for SPR meter
	float formantRatio = minFormantRatio;
	float prevFormantRatio = minFormantRatio;

	// UI Size Stuff
	float meterLength = 450;
	float meterWidth = 50;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MpasAudioProcessorEditor)
};
