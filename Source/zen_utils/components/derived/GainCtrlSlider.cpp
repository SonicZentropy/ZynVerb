/* ==============================================================================
//  GainCtrlSlider.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/08
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Specifically for Gain parameter management.  Handles Log/Linear normalization
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#include "GainCtrlSlider.h"
#include <sstream>
#include <iomanip>
#include "../AssociatedSlider.h"
#include "../../utilities/DecibelConversions.h"

namespace Zen
{

	GainCtrlSlider::GainCtrlSlider(const String& componentName, ZenParameter* associatedParam)
		: AssociatedSlider(componentName, associatedParam)
	{

		setValue(associatedParam->getValue());
	}

	GainCtrlSlider::GainCtrlSlider(const String& componentName, ZenParameter* associatedParam, const String& desiredUnitLabel)
		: AssociatedSlider(componentName, associatedParam)
	{

		setValue(associatedParam->getValue());
		
		this->setTextValueSuffix(desiredUnitLabel);
	}

	GainCtrlSlider::GainCtrlSlider(const String& componentName, ZenParameter* associatedParam, const String& desiredUnitLabel, const float& desiredMaxDBRange)
		: AssociatedSlider(componentName, associatedParam), maximumDecibelsInRange(desiredMaxDBRange)
	{
		setValue(associatedParam->getValue());
		this->setTextValueSuffix(desiredUnitLabel);
	}

	/// <summary>Gets GUI text from a given value by converting the value into Decibels</summary>
	/// <param name="value">Input value where 0 represents -inf dB and 1.0 represents max Decibels in specified range</param>
	/// <returns>String representing the input value in decibelsFS + Unit label</returns>
	String GainCtrlSlider::getTextFromValue(double value)
	{
		float decibelsFromGain = static_cast<float>(DecibelConversions::mapNormalizedValueToDecibels(static_cast<float>(value), -96, 12));
		if (decibelsFromGain <= -96.0f)
		{
			return String("-INF dB");
		} else
		{
			std::stringstream gainRound;
			gainRound.setf(std::ios::fixed, std::ios::floatfield);
			gainRound << std::setprecision(2) << static_cast<float>(decibelsFromGain);

			return String(String(gainRound.str()) + getTextValueSuffix());
		}
	}

	double GainCtrlSlider::getValueFromText(const String& text)
	{

		float rawValue = text.getFloatValue();
		float gainFromDecibels = DecibelConversions::mapDecibelsToProperNormalizedValue(rawValue, -96.0, 12.0, 0.0);

		return gainFromDecibels;
	}

	GainCtrlSlider::~GainCtrlSlider()
	{

	}
}