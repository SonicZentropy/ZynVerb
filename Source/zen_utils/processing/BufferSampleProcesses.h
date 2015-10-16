/* ==============================================================================
//  BufferSampleProcesses.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/08
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: All Static Methods for discrete sample processing live here
//	and are generally called from PluginEditor.cpp
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#ifndef BUFFER_SAMPLE_PROCESSES_H_INCLUDED
#define BUFFER_SAMPLE_PROCESSES_H_INCLUDED
#include <ctime>



namespace Zen
{
	class BufferSampleProcesses
	{
	public:

		/// <summary> Algorithm for processing audio's stereo width.
		///			  Value 0 -> Mono, 100 -> No width adjustment, 200 -> Sides only.</summary>
		/// <param name="leftSample">  Sample from the left audio channel </param>
		/// <param name="rightSample"> Sample from the right audio channel </param>
		/// <param name="widthValue">  Desired stereo width (0-200) </param>
		static void processStereoWidth(float* leftSample, float* rightSample, const float& widthValue);

		/// <summary> Changes sample gain in decibels </summary>
		/// <param name="leftSample">  Sample from the left audio channel </param>
		/// <param name="rightSample"> Sample from the right audio channel </param>
		/// <param name="gainValue">   The gain value (Normalized) </param>
		static void processGain(float* leftSample, float* rightSample, const float& gainValue);

		/// <summary> Invert the left channel's phase </summary>
		/// <param name="leftSample"> Sample from the left audio channel. </param>
		static void processInvertLeftChannel(float* leftSample);

		/// <summary> Invert the right channel's phase </summary>
		/// <param name="rightSample"> Sample from the right audio channel </param>
		static void processInvertRightChannel(float* rightSample);

	/*	static void processExtractMidSignal(float* leftSample, float* rightSample);
		static void processExtractSideSignal(float* leftSample, float* rightSample);*/

		/// <summary> Pan process (Currently using sinusoidal pan law </summary>
		/// <param name="leftSample">   Sample from the left audio channel</param>
		/// <param name="rightSample"> Sample from the right audio channel </param>
		/// <param name="panRatio">    The pan ratio. 0.0 -> 100%L, 0.5 -> Centered, 1.0 -> 100%R </param>
		static void processPanning(float* leftSample, float* rightSample, const float& panRatio);
	private:
		static clock_t inTime;
		static bool shouldPrint;
	};

}
#endif  // BUFFER_SAMPLE_PROCESSES_H_INCLUDED
