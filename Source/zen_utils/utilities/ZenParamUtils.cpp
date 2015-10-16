/* ==============================================================================
//  ZenParamUtils.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/08/16
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Here's some functions for manipulating parameters. Use the inverse functions in 'getParameter' and 'setParameterNotifyingHost'
//  and the non-inverse in 'setParamer'. The warp functions, borrowed from JUCE's slider class allow you to set a value you wish
//  to be represented by 0.5 - useful for frequency control for example.The intStep functions allow integers to be represented
//  and the Bool is self explanatory.Hope these help.
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
=============================================================================*/
#include "JuceHeader.h"
#include "ZenParamUtils.h"
#include "ZenUtils.hpp"


namespace Zen
{

	float ZenParamUtils::warp(float x, float max, float min, float mid)
	{
		float sf = warpCoefficients(max, min, mid);

		float y = exp(log(x) / sf);
		return min + (max - min) * y;
	}

	float ZenParamUtils::inverseWarp(float x, float max, float min, float mid)
	{
		float sf = warpCoefficients(max, min, mid);
		float n = (x - min) / (max - min);

		return pow(n, sf);
	}

	float ZenParamUtils::warpCoefficients(float max, float min, float mid)
	{
		//Create Skew Factor
		return log(0.5f) / log((mid - min)
			/ (max - min));
	}
	
	float ZenParamUtils::normalizeValueLinear(
		const float& value, const float& minValue, const float& maxValue)
	{
		jassert(maxValue > minValue);

		auto normalized = (value - minValue) / (maxValue - minValue);
		jassert(normalized >= -0.00001f && normalized <= 1.0001f);
		return normalized;
	}


	float ZenParamUtils::denormalizeValueLinear(
		float& normalized, const float& minValue, const float& maxValue)
	{
		jassert(normalized >= -0.00001f && normalized <= 1.00001f);
		jassert(maxValue > minValue);
		normalized = getClamped(normalized, 0, 1.0);
		return minValue + normalized * (maxValue - minValue);
	}

	float ZenParamUtils::convertValueToDecibels(const float& gain)
	{
		return Decibels::gainToDecibels(gain, 0.0f);
	}

	float ZenParamUtils::convertDecibelstoValue(const float& gain)
	{
		return Decibels::decibelsToGain(gain, -96.0f);
	}




	/// <summary>Given input value in an arbitrary range, convert it to a linear normalized version warped around a desired midpoint.
	///	Example: Given a decibel value in a decibel range -96 to +12, convert it to a normalized (0.0 to 1.0) value
	/// where the input mappedMidpointOfRange of 0db maps to 0.5 normalized result.</summary>
	/// <param name="inValue"> Input value, between input minimum and maximum (Ex. 4.2 decibels) </param>
	/// <param name="minOfRange"> The minimum possible value of the input range (Ex. -96 decibels) </param>
	/// <param name="maxOfRange"> The maximum possible value of the input range (Ex. +12 decibels) </param>
	/// <param name="mappedMidpointOfRange">Value IN RANGE that should be tied to 0.5 NORMALIZED (Ex. 0.0 decibels)</param>
	/// <returns> Normalized (0.0 to 1.0) value corresponding to the input value</returns>
	float ZenParamUtils::convertValueToWarpedLinearBasedOnMidpoint(const float& inValue, const float& minOfRange, const float& maxOfRange, const float& mappedMidpointOfRange)
	{
		jassert(inValue >= minOfRange && inValue <= maxOfRange && mappedMidpointOfRange >= minOfRange && mappedMidpointOfRange <= maxOfRange);
		//equation parameters that match the piece of paper I wrote this out on :)
		float x, x0, x1, y0, y1;
		if (inValue < 0)
		{
			x0 = 0.0;
			y0 = minOfRange;
			x1 = 0.5;
			y1 = mappedMidpointOfRange;

		} else if (inValue >= 0)
		{
			x0 = 0.5;
			y0 = mappedMidpointOfRange;
			x1 = 1.0;
			y1 = maxOfRange;
		} else
		{
			jassertfalse;
			return -9000.0f;
		}
		if ((y1 - y0) == 0) return 0;  //prevent divide by 0
		x = (((inValue - y0) / (y1 - y0)) * (x1 - x0)) + x0;
		return x;
	}

	/// <summary>Given input value warp-normalized (0.0 to 1.0), convert it back to given range based on midpoint.
	/// Example: Given decibel gain value where 0.5 equates to 0.0dB, map the value back to raw decibels FS.</summary>
	/// <param name="inValue"> The input value, normalized between 0.0 and 1.0 (Ex. 0.7)</param>
	/// <param name="minOfRange"> The minimum possible value of the input range (Ex. -96 decibels) </param>
	/// <param name="maxOfRange"> The maximum possible value of the input range (Ex. +12 decibels) </param>
	/// <param name="mappedMidpointOfRange">Value IN RANGE that should be tied to 0.5 NORMALIZED (Ex. 0.0 decibels)</param>
	/// <returns>Full range value of the input normalized value</returns>
	float ZenParamUtils::convertMidpointWarpedLinearNormalizedValueToRawRangeValue(float inValue, float minOfRange, float maxOfRange, float mappedMidpointOfRange)
	{
		//Account for float truncation errors
		jassert(inValue >= -0.00001f && inValue <= 1.00001f && mappedMidpointOfRange >= minOfRange && mappedMidpointOfRange <= maxOfRange);
		inValue = getClamped(inValue, 0.0f, 1.0f);
		float y, x0, x1, y0, y1;
		if (inValue <= 0.5)
		{
			x0 = 0.0;
			y0 = minOfRange;
			x1 = 0.5;
			y1 = mappedMidpointOfRange;

		} else if (inValue > 0.5)
		{
			x0 = 0.5;
			y0 = mappedMidpointOfRange;
			x1 = 1.0;
			y1 = maxOfRange;

		} else
		{
			jassertfalse;
			return -9000.0f;
		}
		if ((x1 - x0) == 0) return 0; //prevent divide by zero
		y = ((y1 - y0)*((inValue - x0) / (x1 - x0))) + y0;
		return y;
	}
}

