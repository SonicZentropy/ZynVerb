/*==============================================================================
//  LinearSmoothedValue.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/15
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Smoothes value to prevent clicks/pops
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/
#ifndef LINEARSMOOTHEDVALUE_H_INCLUDED
#define LINEARSMOOTHEDVALUE_H_INCLUDED

#include "JuceHeader.h"
#include "../utilities/ZenParamUtils.h"


namespace Zen
{

class LinearSmoothedValue
{
public:
	LinearSmoothedValue() noexcept
		: currentValue(0), target(0), step(0), countdown(0), stepsToTarget(0)
	{
	}

	void reset(double sampleRate, double fadeLengthSeconds) noexcept
	{
		jassert(sampleRate > 0 && fadeLengthSeconds >= 0);
		stepsToTarget = static_cast<int>(std::floor(fadeLengthSeconds * sampleRate));
		currentValue = target;
		countdown = 0;
	}

	void setValue(float newValue) noexcept
	{
		if (!areAlmostEqual(target, newValue))
		{
			target = newValue;
			countdown = stepsToTarget;

			if (countdown <= 0)
				currentValue = target;
			else
				step = (target - currentValue) / static_cast<float>(countdown);
		}
	}

	float getNextValue() noexcept
	{
		if (countdown <= 0)
			return target;

		--countdown;
		currentValue += step;
		return currentValue;
	}

private:
	float currentValue, target, step;
	int countdown, stepsToTarget;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LinearSmoothedValue)
};
}

#endif // LINEARSMOOTHEDVALUE_H_INCLUDED