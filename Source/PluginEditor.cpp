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
    setSize (230, 550);
	startTimerHz(120); // Timer freq

	zeromem(prevBufferData, sizeof(prevBufferData)); 
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

	// DEBUG 
	/*
	g.drawFittedText(String(lowPartial), getLocalBounds(), Justification::bottomLeft, 1); // Display formant ratio
	g.drawFittedText(String(highPartial), getLocalBounds(), Justification::bottomRight, 1); // Display formant ratio
	*/

	g.drawFittedText("Singing Power Ratio Meter", 70, 20, 90, 30, Justification::centredBottom, 2); // Display Title

	// Paint SPR level
	g.drawLine(60, 
		70 + roundFloatToInt(formantRatio * meterLength / minFormantRatio),
		140, 
		70 + roundFloatToInt(formantRatio * meterLength / minFormantRatio));


	g.setColour(Colours::white);
	// Draw Meter
	// Border
	g.drawLine(70, 70, 70, meterLength + 70);
	g.drawLine(120, 70, 120, meterLength + 70);

	// Dashes
	// 0dB
	g.drawLine(70, 70, 78, 70);
	g.drawLine(112, 70, 120, 70);
	g.drawFittedText("0dB", 81, 65, 28, 10, Justification::centred, 1);
	// 5dB
	g.drawLine(70, 0.111 * meterLength + 70, 78, 0.111 * meterLength + 70);
	g.drawLine(112, 0.111 * meterLength + 70, 120, 0.111 * meterLength + 70);
	g.drawFittedText("5dB", 81, 0.111 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 10dB
	g.drawLine(70, 0.222 * meterLength + 70, 78, 0.222 * meterLength + 70);
	g.drawLine(112, 0.222 * meterLength + 70, 120, 0.222 * meterLength + 70);
	g.drawFittedText("10dB", 81, 0.222 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 15dB
	g.drawLine(70, 0.333 * meterLength + 70, 78, 0.333 * meterLength + 70);
	g.drawLine(112, 0.333 * meterLength + 70, 120, 0.333 * meterLength + 70);
	g.drawFittedText("15dB", 81, 0.333 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 20dB
	g.drawLine(70, 0.444 * meterLength + 70, 78, 0.444 * meterLength + 70);
	g.drawLine(112, 0.444 * meterLength + 70, 120, 0.444 * meterLength + 70);
	g.drawFittedText("20dB", 81, 0.444 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 25dB
	g.drawLine(70, 0.556 * meterLength + 70, 78, 0.556 * meterLength + 70);
	g.drawLine(112, 0.556 * meterLength + 70, 120, 0.556 * meterLength + 70);
	g.drawFittedText("25dB", 81, 0.556 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 30dB
	g.drawLine(70, 0.667 * meterLength + 70, 78, 0.667 * meterLength + 70);
	g.drawLine(112, 0.667 * meterLength + 70, 120, 0.667 * meterLength + 70);
	g.drawFittedText("30dB", 81, 0.667 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 35dB
	g.drawLine(70, 0.778 * meterLength + 70, 78, 0.778 * meterLength + 70);
	g.drawLine(112, 0.778 * meterLength + 70, 120, 0.778 * meterLength + 70);
	g.drawFittedText("35dB", 81, 0.778 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 40dB
	g.drawLine(70, 0.889 * meterLength + 70, 78, 0.889 * meterLength + 70);
	g.drawLine(112, 0.889 * meterLength + 70, 120, 0.889 * meterLength + 70);
	g.drawFittedText("40dB", 81, 0.889 * meterLength + 65, 28, 10, Justification::centred, 1);
	// 45dB
	g.drawLine(70, meterLength + 70, 78, meterLength + 70);
	g.drawLine(112, meterLength + 70, 120, meterLength + 70);
	g.drawFittedText("45dB", 81, meterLength + 65, 28, 10, Justification::centred, 1);

	// Draw good/bad
	g.drawFittedText("Good", 150, 0.25 * meterLength + 65, 70, 10, Justification::left, 1);
	g.drawFittedText("Bad", 150, 0.75 * meterLength + 65, 70, 10, Justification::left, 1);
	// Good bracket
	g.drawLine(140, 70, 140, 70 + (meterLength / 2) - 2);
	g.drawLine(140, 70, 148, 70);
	g.drawLine(140, 70 + (meterLength / 2) - 2, 148, 70 + (meterLength / 2) - 2);
	//Bad bracket
	g.drawLine(140, 70 + (meterLength / 2) + 2, 140, 70 + meterLength);
	g.drawLine(140, 70 + (meterLength / 2) + 2, 148, 70 + (meterLength / 2) + 2);
	g.drawLine(140, 70 + meterLength, 148, 70 + meterLength);

	// DEBUG - Visual aid when unvoiced audio detected
	/*
	if ((processor.numZeroCrossing >= 40 && processor.numZeroCrossing != 0) || maxACF < pow(10, -3))
		g.setColour(Colours::red);

	g.fillRect(130, 320, 10, 10);
	*/
}

void MpasAudioProcessorEditor::timerCallback() {
	// This call function gets called periodically

	if (processor.nextFFTBlockReady)
	{
		// Copy fftData array in processor to local array
		for (int i = 0; i < (2 * fftSize); ++i)
		{	
			fftData[i] = processor.fftData[i];
			bufferData[i] = processor.bufferData[i];
		}

		// Copy buffer in processor to local array
		// This seperate buffer is for calculating the maximum auto-corelation function
		float peakValue = 0;
		for (int i = 0; i < (fftSize); ++i)
		{
			bufferData[i] = processor.bufferData[i]; 

			// Find largest value in data buffers (for some rough normalization when doing auto-correlation later)
			if (abs(bufferData[i]) > peakValue)
				peakValue = abs(bufferData[i]);
			if (abs(prevBufferData[i]) > peakValue)
				peakValue = abs(prevBufferData[i]);
		}

		normalizationMultiplier = double(1 / peakValue);
		// Nomalize buffer data arrays
		// If multiplier is too large, then assume it's just background noise and don't multiply
		if (normalizationMultiplier < 1000)
			for (int i = 0; i < (fftSize); ++i)
			{
				bufferData[i] *= normalizationMultiplier;
				prevBufferData[i] *= normalizationMultiplier;
			}

		// Find Maximum of Autocorrelation Function
		float ACF;
		maxACF = 0;
		for (int i = 0; i < (fftSize-1); ++i)
		{
			// Calculate ACF 
			ACF = 0;
			for (int idx = 0; idx < (fftSize); ++idx)
			{
				ACF += bufferData[idx] * prevBufferData[idx];
			}

			// Get magnitude
			ACF = abs(ACF/ fftSize); 
			if (maxACF < ACF)
				maxACF = double(ACF);

			// Decrease size of delay beteen prevBuffer and current buffer before looping back
			for (int idx = 0; idx < (fftSize - 1); ++idx)
			{
				prevBufferData[idx] = prevBufferData[idx + 1];
			}
			prevBufferData[fftSize] = bufferData[i];
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

		lowPartial = Decibels::gainToDecibels(lowPartial);
		highPartial = Decibels::gainToDecibels(highPartial);

		formantRatio = lowPartial - highPartial;

		// Set limits
		if (formantRatio <= 0)
			formantRatio = 0;

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

		// Set to bottom of meter (In this case, 45db, which is the bottom of meter) when unvoiced.
		// This is done using zero crossing values and auto-correlation
		// Spectral flux is also accounted for, to help ignore transition between notes.
		if ((processor.numZeroCrossing >= 40 && processor.numZeroCrossing != 0) || maxACF < pow(10, -3))
			formantRatio = minFormantRatio;

		// Ramp down
		if (prevFormantRatio + 1 < formantRatio) 
			if (prevFormantRatio >= minFormantRatio - 1)
				formantRatio = minFormantRatio;
			else
				formantRatio = prevFormantRatio + 1;
		
		if (prevFormantRatio - 3  > formantRatio) // Ramp Up
			if (prevFormantRatio <=  - 3)
				formantRatio = 0;
			else
				formantRatio = prevFormantRatio - 3;

		// Save data for next frame
		prevFormantRatio = formantRatio;

		for (int i = 0; i < (2 * fftSize); ++i)
		{
			prevFFTData[i] = fftData[i];
		}

		for (int i = 0; i < (fftSize); ++i)
		{
			prevBufferData[i] = processor.bufferData[i];
		}

		repaint(); // Refresh GUI

	}


}

void MpasAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

