// Filterblock.h: header file for the first order Filterblock class

#pragma once
#include <vector>

class Filterblock {
public:
	Filterblock() {} // Default constructor
	
	void setup();
	
	// Sets the coefficients after they are caclculated
	void setCoefficients(float gb0, float gb1, float ga1);
	
	// Apply the coefficients to the input and update the state
	float process(float in);		
	
	~Filterblock() {}				// Destructor

private:
	// coefficients and state varaibles
	// for an instance of a first order Filterblock
	float gB0;
	float gB1;
	float gA1;
	float gLastX1=0;
	float gLastY1=0;

};