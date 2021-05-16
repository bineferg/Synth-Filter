// Queen Mary University of London
// ECS7012 - Music and Audio Programming
// Spring 2021
//
// Assignment 1: Synth Filter
// This project contains template code for implementing a resonant filter with
// parameters adjustable in the Bela GUI, and waveform visible in the scope

#include <Bela.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include <libraries/Scope/Scope.h>
#include <libraries/math_neon/math_neon.h>
#include <cmath>
#include "Wavetable.h"
#include "Filterblock.h"
#include <cstdlib>

// Browser-based GUI to adjust parameters
Gui gGui;
GuiController gGuiController;

// Browser-based oscilloscope to visualise signal
Scope gScope;

// Oscillator objects
Wavetable gSineOscillator, gSawtoothOscillator;

// Filterblock Objects
Filterblock block1, block2, block3, block4;

// ****************************************************************
// TODO: declare your global variables here for coefficients and filter state
// ****************************************************************

// Initialise global coeffient variables
float gA1 = 0;
float gB0 = 0, gB1 = 0;

// Initalise block and sample state (depending on the step of the lab)
float gLastX1 = 0;
float gLastY1 = 0;

// Initalise global Res and Comp variables for resonance
float gRes = 0.75;
float gComp = 0.5;

// Calculate filter coefficients given specifications
// frequencyHz -- filter frequency in Hertz (needs to be converted to discrete time frequency)
// resonance -- normalised parameter 0-1 which is related to filter Q
void calculate_coefficients(float sampleRate, float frequencyHz, float resonance)
{
	// ******************************************************************
	// TODO: calculate filter coefficients following the assignment brief 
	// ******************************************************************
	float wc = (2.0 * M_PI * frequencyHz) / sampleRate;
	//float g = (2.0 * M_PI * frequencyHz) / sampleRate; // Uncomment for the previous labe step
	float g = 0.9892*wc - 0.4342*powf(wc,2) + 0.1381*powf(wc, 3) - 0.0202*powf(wc,4);
	
	// Updated calculation of resonant value
	gRes = resonance*(1.0029 + (0.0526*wc) - 0.0926*powf(wc,2) + 0.0218*powf(wc,3));
    
    // Calculation of coefficients based on g
    gA1 = 1-g;
    gB0 = (1.0 * g / 1.3);
    gB1 = (0.3 * g /1.3);
}


bool setup(BelaContext *context, void *userData)
{
	std::vector<float> wavetable;
	const unsigned int wavetableSize = 1024;
		
	// Populate a buffer with the first 64 harmonics of a sawtooth wave
	wavetable.resize(wavetableSize);
	
	// Generate a sawtooth wavetable (a ramp from -1 to 1)
	for(unsigned int n = 0; n < wavetableSize; n++) {
		wavetable[n] = -1.0 + 2.0 * (float)n / (float)(wavetableSize - 1);
	}
	
	// Initialise the sawtooth wavetable, passing the sample rate and the buffer
	gSawtoothOscillator.setup(context->audioSampleRate, wavetable);

	// Recalculate the wavetable for a sine
	for(unsigned int n = 0; n < wavetableSize; n++) {
		wavetable[n] = sin(2.0 * M_PI * (float)n / (float)wavetableSize);
	}
	
	// Set up the filter blocks used for the fourth order series
	block1.setup();
	block2.setup();
	block3.setup();
	block4.setup();
	
	// Initialise the sine oscillator
	gSineOscillator.setup(context->audioSampleRate, wavetable);

	// Set up the GUI
	gGui.setup(context->projectName);
	gGuiController.setup(&gGui, "Oscillator and Filter Controls");	
	
	// Arguments: name, default value, minimum, maximum, increment
	// Create sliders for oscillator and filter settings
	gGuiController.addSlider("Oscillator Frequency", 440, 40, 8000, 1);
	gGuiController.addSlider("Oscillator Amplitude", 0.5, 0, 2.0, 0);
	gGuiController.addSlider("Filter Cutoff Frequency", 1000, 100, 5000, 1);
	gGuiController.addSlider("Filter Resonance", .25, 0, 1.1, .01);
	
	// Set up the scope
	gScope.setup(2, context->audioSampleRate);

	return true;
}

void render(BelaContext *context, void *userData)
{
	// Read the slider values
	float oscFrequency = gGuiController.getSliderValue(0);	
	float oscAmplitude = gGuiController.getSliderValue(1);
	float filterCutoffFrequency = gGuiController.getSliderValue(2);
	float cResonance = gGuiController.getSliderValue(3);

	// Set the oscillator frequency
	gSineOscillator.setFrequency(oscFrequency);
	gSawtoothOscillator.setFrequency(oscFrequency);

	// Calculate new filter coefficients
	calculate_coefficients(context->audioSampleRate, filterCutoffFrequency, cResonance);
	block1.setCoefficients( gB0, gB1, gA1);
	block2.setCoefficients( gB0, gB1, gA1);
	block3.setCoefficients( gB0, gB1, gA1);
	block4.setCoefficients( gB0, gB1, gA1);

    for(unsigned int n = 0; n < context->audioFrames; n++) {
    	// Uncomment one line or the other to choose sine or sawtooth oscillator
    	// (or, if you like, add a GUI or hardware control to switch on the fly)
		//float in = oscAmplitude * gSineOscillator.process();
		float in = oscAmplitude * gSawtoothOscillator.process();
            
        // ****************************************************************
        // TODO: apply the filter to the input signal
        
        // First order IIR filter (with feedback)
        //float out = gB0*in + gB1 * gLastX1 + gA1 * gLastY1;
		// gLastX1 = in;
        // gLastY1 = out; 
        
        // Adding feedback to the input
        float feedIn = (1+4*gRes*gComp)*in - 4*gRes*gLastY1;
		
		// Adding non-linearity to the filter
		float tanIn= tanhf_neon(feedIn);
		
		// Fourth order IIR filter (with feedback) used as instance blocks
		float out1 = block1.process(tanIn);		
		float out2 = block2.process(out1);
		float out3 = block3.process(out2);
		float out4 = block4.process(out3);
		
		// Update gLastY1 to maintaint the feedback loop
		gLastY1 = out4;
        // ****************************************************************
            
        // Write the output to every audio channel
    	for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
    		audioWrite(context, n, channel, out4);
    	}
    	
    	gScope.log(in, out4);
    }
}

void cleanup(BelaContext *context, void *userData)
{

}
