/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MpasAudioProcessorEditor::MpasAudioProcessorEditor (MpasAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), 
	forwardFFT(processor.fftOrder), // FFT constructor
	window(fftSize, dsp::WindowingFunction<float>::hann)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
	startTimerHz(20); // Timer freq
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
    g.drawFittedText (String(Decibels::gainToDecibels(formantRatio)), getLocalBounds(), Justification::centred, 1); // Display formant ratio
	//g.drawFittedText(String(Decibels::gainToDecibels(lowPartial) - Decibels::gainToDecibels(highPartial)), getLocalBounds(), Justification::centred, 1); // Display formant ratio
	g.drawFittedText (String((lowPartial)), getLocalBounds(), Justification::bottomLeft, 1); // Display formant ratio
	g.drawFittedText(String((highPartial)), getLocalBounds(), Justification::bottomRight, 1); // Display formant ratio
}

void MpasAudioProcessorEditor::timerCallback() {
	// This call function gets called periodically

	if (processor.nextFFTBlockReady)
	{
		// Copy fftData array in processor to local array
		float fftData[2 * fftSize];
		for (int i = 0; i < (2 * fftSize); ++i)
		{	
			fftData[i] = processor.fftData[i];
			//fftData[i] = 1;
		}

		debug = fftData[55];
		// Apply window
		window.multiplyWithWindowingTable(fftData, fftSize);

		// Render FFT data 
		forwardFFT.performFrequencyOnlyForwardTransform(fftData);

		processor.nextFFTBlockReady = false;

		lowPartial = 0;
		for (auto idx = 0; idx < lowFreqsLimIdx; ++idx)
		{
			if (fftData[idx] > lowPartial)
				lowPartial = fftData[idx];
		}

		highPartial = 0;
		for (auto idx = lowFreqsLimIdx; idx < highFreqsLimIdx; ++idx)
		{
			if (fftData[idx] > highPartial)
				highPartial = fftData[idx];
		}
		//highFreqsPowDensity = highFreqsPowDensity / (highFreqsLimIdx - lowFreqsLimIdx);

		//formantRatio = highFreqsPowDensity / lowFreqsPowDensity;
		//formantRatio = Decibels::gainToDecibels(lowPartial)/ Decibels::gainToDecibels(highPartial);
		formantRatio = lowPartial - highPartial;
		//formantRatio = ceilf(formantRatio * 100) / 100;      /* Round up*/

		repaint(); // Refresh GUI

	}


}

void MpasAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

