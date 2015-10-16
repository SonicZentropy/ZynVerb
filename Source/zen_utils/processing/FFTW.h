/*
==============================================================================

This file was auto-generated by the Introjucer!

It contains the basic framework code for a JUCE plugin processor.

==============================================================================
*/

#ifndef INPLACEFFTW_H_INCLUDED
#define INPLACEFFTW_H_INCLUDED

#include <fftw3.h>
#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
FFTW wrapper

Optimized FFT using the FFTW libraries. Does out-of-place
real-to-complex or complex-to-real Fourier-Transforms.
*/
class FFTW
{
public:
	/**
	Initialises an object for performaing an FFT. This will call the
	fftw create plan functions and can incur an overhead.	This will
	allocate the necessary input and output buffers which can be accessed
	by getRealBuffer() and getComplexBuffer() functions.
	time_domain_size need NOT be a power of two as with juce's FFT.
	*/
	FFTW(unsigned int time_domain_size, bool forwardDirection);

	/**
	Destroy the FFT and release the associated buffers.
	*/
	~FFTW();

	/**
	Execute the FFT.
	*/
	inline void execute()
	{
		fftwf_execute(plan);
	}

	inline int getRealSize() const
	{
		return size;
	}

	inline int getComplexSize() const
	{
		return (size >> 1) + 1;
	}

	inline FFT::Complex * getComplexBuffer()
	{
		return complexBuffer;
	}

	inline float * getRealBuffer()
	{
		return realBuffer;
	}
private:
	int size;
	float* realBuffer;
	FFT::Complex* complexBuffer;
	fftwf_plan plan;
};

#endif // INPLACEFFTW_H_INCLUDED
