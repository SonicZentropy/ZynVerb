/* ==============================================================================
//  ZenDebugUtils.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/08
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Static Utility Class for Debug-related code
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/
#include "JuceHeader.h"
#include "ZenDebugUtils.h"


namespace Zen
{

clock_t ZenDebugUtils::inTime = clock();

int ZenDebugUtils::numSecondsBetweenDebugPrints = 1;

void ZenDebugUtils::setSecondsBetweenDebugPrints(const unsigned int& inSeconds)
{
	numSecondsBetweenDebugPrints = inSeconds;
}

int ZenDebugUtils::getSecondsBetweenDebugPrints()
{
	return numSecondsBetweenDebugPrints;
}

bool ZenDebugUtils::isPrintTimerThresholdReached()
{
	if (((clock() - inTime) / CLOCKS_PER_SEC) > numSecondsBetweenDebugPrints)
	{
		inTime = clock();
		return true;
	}
	return false;
}

void ZenDebugUtils::timedPrint(String inString)
{
	if (isPrintTimerThresholdReached())
	{
		DBG(inString);
	}
}

} // Namespace