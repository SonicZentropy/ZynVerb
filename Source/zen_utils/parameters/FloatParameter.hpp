/* ==============================================================================
//  FloatParameter.cpp
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/08/18
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Derived AudioProcessorParameter for Float values.  Pure
//  virtual methods that must be overridden:
//   destructor
//   getValue
//   setValue
//   getDefaultValue
//   getName
//   getLabel (units string, example "Hz" or "%")
//   getValueForText (parse a String and return appropriate value for it)
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/
#ifndef ZEN_FLOAT_PARAMETER_H_INCLUDED
#define ZEN_FLOAT_PARAMETER_H_INCLUDED

#include "JuceHeader.h"
#include "ZenParameter.hpp"



namespace Zen
{
using namespace juce;
class FloatParameter : public ZenParameter
{
public:

	FloatParameter()
	{
		std::logic_error("Float Parameter Default Constructor should never be called");
		jassertfalse;
	}

	FloatParameter(const String& paramName, const float& inDefaultValue, const bool& inShouldBeSmoothed = false, const float& smoothingTime = 0.01f, const String& inLabel = "")
		: ZenParameter(paramName, inDefaultValue, inShouldBeSmoothed, smoothingTime, inLabel)
	{
//		DBGM("In FloatParameter::FloatParameter() ");
	}

	explicit FloatParameter(const String& inParameterName, const float& inMinValue = 0.0f, const float& inMaxValue = 1.0f, const float& inDefaultValue = 0.5f, 
		           const float& inStep = 0.01f, const bool& inShouldBeSmoothed = false, const float& smoothingTime = 0.01f, const String& inLabel = "")
		: ZenParameter(inParameterName, inMinValue, inMaxValue, inDefaultValue, inStep, inShouldBeSmoothed, smoothingTime, inLabel)
	{
//		DBGM("In FloatParameter::FloatParameter() ");
		range = inMaxValue - inMinValue;
	}

	virtual ~FloatParameter()
	{
//		DBGM("In FloatParameter::~FloatParameter() ");
	}

	virtual float getScaledValue() const
	{
		return (getValue() - getMinValue()) / range;
	}

	virtual void setScaledValue(const float& inValue)
	{
		setValue(inValue * range + getMinValue());
	}

protected:
	
	float range=1.0f;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatParameter);

};
}//namespace
#endif   // ZEN_FLOAT_PARAMETER_H_INCLUDED