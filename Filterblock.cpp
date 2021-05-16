// Filterblock.cpp: file for implementing the first order Filterblock class

#include <cmath>
#include "Filterblock.h"

// Setup initliaising coefficients to 0
void Filterblock::setup()
{
	gB0 = 0;
	gB1 = 0;
	gA1 = 0;
}


// Apply the coefficients to the state
float Filterblock::process(float in) {
	float out = gB0*in + gB1 * gLastX1 + gA1 * gLastY1;
	gLastX1 = in;
    gLastY1 = out; 
    return out;
	
}	

// Set the coefficients to those calculated in calculate_coefficients
void Filterblock::setCoefficients(float gb0, float gb1, float ga1) {
	gB0 = gb0;
	gB1 = gb1;
	gA1 = ga1;
}