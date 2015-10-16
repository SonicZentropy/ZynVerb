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

#ifndef ZEN_PARAM_UTILS_H_INCLUDED
#define ZEN_PARAM_UTILS_H_INCLUDED

namespace Zen
{

/// <summary>
/// Generic Utilities for dealing with JUCE Parameters
/// </summary>
	class ZenParamUtils
	{
	public:
		/// <summary>
		/// Warps Audio normalization from juce_zen_utils.h 
		/// fixed. Use in setParameter
		/// </summary>
		/// <param name="x">Incoming Value</param>
		/// <param name="max">Max of range</param>
		/// <param name="min">Min of Range</param>
		/// <param name="mid">Midpoint for 0.5 skew</param>
		/// <returns>Post-normalized value</returns>
		static float warp(float x, float max, float min, float mid);

		/// Warps Audio normalization from juce_zen_utils.h 
		/// fixed. Use in setParameterNotifyingHost and getParameter
		/// <param name="x">  Incoming value </param>
		/// <param name="max"> Max of range </param>
		/// <param name="min"> Minimum of range </param>
		/// <param name="mid"> The middle of range </param>
		/// <returns>Re-warped value </returns>
		static float inverseWarp(float x, float max, float min, float mid);

		/// <summary>
		/// Returns skew coeff
		/// </summary>
		/// <param name="max">The maximum.</param>
		/// <param name="min">The minimum.</param>
		/// <param name="mid">The mid.</param>
		/// <returns>float.</returns>
		static float warpCoefficients(float max, float min, float mid);

		/// <summary>
		/// Given a float value normalized in the range 0-1, return the value linearly rescaled/denormalized
		/// to the range of (float) min - max
		/// </summary>
		/// <param name="value">Input Value from arbitrary range (GUI Input Methods)</param>
		/// <param name="minValue">Minimum of GUI value's range</param>
		/// <param name="maxValue">Maximum of GUI value's range</param>
		/// <returns></returns>
		static float normalizeValueLinear(const float& value, const float& minValue, const float& maxValue);

		/// <summary>
		/// Given a denormalized float variable plus range min/max, return a normalized 0-1 value
		/// </summary>
		/// <param name="normalized">The normalized input value</param>
		/// <param name="minValue">Minimum value of desired range</param>
		/// <param name="maxValue">Maximum value of desired range</param>
		/// <returns>(float) Denormalized value between given min/max</returns>
		static float denormalizeValueLinear(float& normalized, const float& minValue, const float& maxValue);


		static float convertValueToDecibels(const float& gain);
		static float convertDecibelstoValue(const float& gain);
	
		/// <summary>Given input value in an arbitrary range, convert it to a linear normalized version warped around a desired midpoint.
		///	Example: Given a decibel value in a decibel range -96 to +12, convert it to a normalized (0.0 to 1.0) value
		/// where the input midPoint of 0db maps to 0.5 normalized result.</summary>
		/// <param name="inValue"> Input value, between input minimum and maximum (Ex. 4.2 decibels) </param>
		/// <param name="minOfRange"> The minimum possible value of the input range (Ex. -96 decibels) </param>
		/// <param name="maxOfRange"> The maximum possible value of the input range (Ex. +12 decibels) </param>
		/// <param name="midPoint">Value IN RANGE that should be tied to 0.5 NORMALIZED (Ex. 0.0 decibels)</param>
		/// <returns> Normalized (0.0 to 1.0) value corresponding to the input value</returns>
		static float convertValueToWarpedLinearBasedOnMidpoint(const float& inValue, const float& minOfRange, const float& maxOfRange, const float& midPoint);
		
		/// <summary>Given input value warp-normalized (0.0 to 1.0), convert it back to given range based on midpoint. Example: Given decibel gain value where 0.5 equates to 0.0dB, map the value back to raw decibels FS.</summary>
		/// <param name="inValue"> The input value, normalized between 0.0 and 1.0 (Ex. 0.7)</param>
		/// <param name="minOfRange"> The minimum possible value of the input range (Ex. -96 decibels) </param>
		/// <param name="maxOfRange"> The maximum possible value of the input range (Ex. +12 decibels) </param>
		/// <param name="mappedMidpointOfRange">Value IN RANGE that should be tied to 0.5 NORMALIZED (Ex. 0.0 decibels)</param>
		/// <returns>Full range value of the input normalized value</returns>
		static float convertMidpointWarpedLinearNormalizedValueToRawRangeValue(float inValue, float minOfRange, float maxOfRange, float mappedMidpointOfRange);
	private:
		ZenParamUtils() {};

	};
}  // Zen Namespace


namespace
{
inline bool areAlmostEqual(const double& x, const double& y, const double& precision = 0.00001)
{
	return (x - y < precision || y - x < precision) ? false : true;
}

inline bool almostEqual(const double& x, const double& y, const double& precision = 0.00001)
{
	return (x - y < precision || y - x < precision) ? false : true;
}

/// <summary> Convert float to boolean. Float 0.0 => False, any other value True </summary>
inline bool convertFloatToBoolean(const float& inFloat)
{
	return (0.0 == inFloat) ? false : true;
}

/// <summary>Convert Boolean to float. True => 1.0, False => 0.0</summary>
inline float convertBooleanToFloat(const bool& inBool)
{
	return (false == inBool) ? 0.0 : 1.0;
}

/// <summary> Convert a 0-1.0 normalized value into a boolean where 0.5+ is true </summary>
inline bool convertFloatToBooleanViaRange(float inputValue)
{
	if (inputValue < 0.5)
		return false;
	else
		return true;
}

} // anon namespace

#endif //ZEN_PARAM_UTILS_H_INCLUDED
