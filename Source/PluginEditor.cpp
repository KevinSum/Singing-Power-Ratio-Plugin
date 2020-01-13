#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
MpasAudioProcessorEditor::MpasAudioProcessorEditor (MpasAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), 
	forwardFFT(processor.fftOrder), // FFT constructor
	window(fftSize, dsp::WindowingFunction<float>::blackman),// Window for fft 
	smoothedValue(10.0)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (230, 500);
	startTimerHz(120); // Timer freq
}

MpasAudioProcessorEditor::~MpasAudioProcessorEditor()
{
}

//==============================================================================
void MpasAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);

	// DEBUG ------
	g.drawFittedText(String(actualFormant), getLocalBounds(), Justification::bottomLeft, 1); // Display formant ratio
	//g.drawFittedText(String(spectralFlux * 1000), getLocalBounds(), Justification::bottomRight, 1); // Display formant ratio
	// ------------------

	g.drawFittedText("Singing Power Ratio Meter", 70, 20, 90, 30, Justification::centredBottom, 2); // Display formant ratio

	// Paint SPR level
	g.drawLine(60, 
		70 + roundFloatToInt((formantRatio - maxFormantRatio) * meterLength / (minFormantRatio - maxFormantRatio)),
		140, 
		70 + roundFloatToInt((formantRatio - maxFormantRatio) * meterLength / (minFormantRatio - maxFormantRatio)));


	g.setColour(Colours::white);
	// Draw Meter
	// Border
	g.drawLine(70, 70, 70, meterLength + 70);
	g.drawLine(120, 70, 120, meterLength + 70);

	// Dashes
	// 15dB
	g.drawLine(70, 70, 78, 70);
	g.drawLine(112, 70, 120, 70);
	g.drawFittedText("15dB", 81, 65, 28, 10, Justification::centred, 1);
	// 20dB
	g.drawLine(70, 0.2 * meterLength + 70, 78, 0.2 * meterLength + 70);
	g.drawLine(112, 0.2 * meterLength + 70, 120, 0.2 * meterLength + 70);
	g.drawFittedText("20dB", 81, 0.2 * meterLength + 65, 28, 10, Justification::centred, 1);
	g.drawFittedText("Good", 127, 0.25 * meterLength + 65, 70, 10, Justification::left, 1);
	// 25dB
	g.drawLine(70, 0.4 * meterLength + 70, 78, 0.4 * meterLength + 70);
	g.drawLine(112, 0.4 * meterLength + 70, 120, 0.4 * meterLength + 70);
	g.drawFittedText("25dB", 81, 0.4 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 30dB
	g.drawLine(70, 0.6 * meterLength + 70, 78, 0.6 * meterLength + 70);
	g.drawLine(112, 0.6 * meterLength + 70, 120, 0.6 * meterLength + 70);
	g.drawFittedText("30 dB", 81, 0.6 * meterLength + 65, 28, 10, Justification::centred, 1);
	g.drawFittedText("Poor", 127, 0.75 * meterLength + 65, 70, 10, Justification::left, 1);
	// 35dB
	g.drawLine(70, 0.8 * meterLength + 70, 78, 0.8 * meterLength + 70);
	g.drawLine(112, 0.8 * meterLength + 70, 120, 0.8 * meterLength + 70);
	g.drawFittedText("35 dB", 81, 0.8 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 40dB
	g.drawLine(70, meterLength + 70, 78, meterLength + 70);
	g.drawLine(112, meterLength + 70, 120, meterLength + 70);
	g.drawFittedText("40 dB", 81, meterLength + 65, 28, 10, Justification::centred, 1);

	// Draw good/bad
	g.drawLine(140, 70, 140, 70 + (meterLength / 2) - 2);
	g.drawLine(140, 70, 148, 70);
	g.drawLine(140, 70 + (meterLength / 2) - 2, 148, 70 + (meterLength / 2) - 2);

	g.drawLine(140, 70 + (meterLength / 2) + 2, 140, 70 + meterLength);

	// TEST

	if (processor.numZeroCrossing >= 20 || formantRatio <= 0 || energy < 0.2 || spectralFlux > 0.2)
		g.setColour(Colours::red);

	//g.fillRect(130, 320, 10, 10);
}

void MpasAudioProcessorEditor::timerCallback() {
	// This call function gets called periodically

	if (processor.nextFFTBlockReady)
	{
		// Copy fftData array in processor to local array
		float fftData[2 * fftSize];
		energy = 0; // Total energy of array
		for (int i = 0; i < (2 * fftSize); ++i)
		{	
			fftData[i] = processor.fftData[i];
			energy = energy + pow(fftData[i], 2);
		}

		// Apply window
		window.multiplyWithWindowingTable(fftData, fftSize);

		// Render FFT data 
		forwardFFT.performFrequencyOnlyForwardTransform(fftData);

		processor.nextFFTBlockReady = false;

		// Calculate singing power ratio
		lowPartial = 0;
		for (auto idx = 0; idx < lowFreqsLimIdx; ++idx) // Go through frequency data below 2kHz
		{
			if (fftData[idx] > lowPartial) // Find highest partial
				lowPartial = fftData[idx]; 
		}

		highPartial = 0;
		for (auto idx = lowFreqsLimIdx; idx < highFreqsLimIdx; ++idx)
		{
			if (fftData[idx] > highPartial)
				highPartial = fftData[idx];
		}
	
		formantRatio = lowPartial - highPartial;

		actualFormant = formantRatio;

		formantRatio = Decibels::gainToDecibels(formantRatio);

		// Set limits
		// Ignore values below 0. This will be set to the lower bound instead. This implies the higher
		// formant has a higher partial than the lower formant, which shouldn't happen with voice
		if (formantRatio < maxFormantRatio && formantRatio > 0)
			formantRatio = maxFormantRatio;

		if (formantRatio > minFormantRatio)
			formantRatio = minFormantRatio;

		// Spectral flux
		spectralFlux = 0;
		for (int i = 0; i < fftSize; ++i)
		{
			spectralFlux = pow(abs(fftData[i]) - abs(prevFFTData[i]), 2);
		}
		spectralFlux = sqrt(spectralFlux);
		spectralFlux = spectralFlux / 2;

		// Set to bottom of meter (In this case, 45db, which is the bottom of meter) when unvoiced
		// or when ratio is lower than 0 (Which should never happen with voices)
		// Spectral flux is also accounted for, to help ignore transition between notes.
		if (processor.numZeroCrossing >= 20 || formantRatio <= 0 || energy < 0.2 || spectralFlux > 0.2)
			formantRatio = minFormantRatio;

		// Ramp
		if (prevFormantRatio + 1 < formantRatio) 
			if (prevFormantRatio >= minFormantRatio - 1)
				formantRatio = minFormantRatio;
			else
				formantRatio = prevFormantRatio + 1;
		
		if (prevFormantRatio - 3  > formantRatio) // Ramp Up
			if (prevFormantRatio <= maxFormantRatio - 3)
				formantRatio = maxFormantRatio;
			else
				formantRatio = prevFormantRatio - 3;

		// Save data for next frame
		prevFormantRatio = formantRatio;

		for (int i = 0; i < (2 * fftSize); ++i)
		{
			prevFFTData[i] = fftData[i];
		}

		// TEST ZERO CROSSING STUFF
		/*
		if (zeroDisplayDebug - 2 > processor.numZeroCrossing)
			zeroDisplayDebug = zeroDisplayDebug - 5;
		else
			zeroDisplayDebug = processor.numZeroCrossing;
		*/

		repaint(); // Refresh GUI

	}


}

void MpasAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

