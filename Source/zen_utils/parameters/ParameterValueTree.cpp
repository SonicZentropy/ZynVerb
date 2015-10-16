/*==============================================================================
//  ParameterValueTree.cpp
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/20
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Constructs and maintains a ValueTree representation of a parameter and its associated data
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#include "ParameterValueTree.h"
#include "..\utilities\ZenParamUtils.h"
namespace Zen
{
	//paramValueTree = new ParameterValueTree(name, value.getValueSource(), defaultValue, shouldBeSmoothed);
	
ParameterValueTree::ParameterValueTree()
{
	DBG("In ParameterValueTree::ParameterValueTree() - Should never be called");
	jassertfalse;
}

ParameterValueTree::ParameterValueTree(String inName, Value parameterValueReference, float inDefaultValue, bool inShouldBeSmoothed)
	: parameterName(inName),
	parameterValue(parameterValueReference),
	defaultValue(inDefaultValue),
	isSmoothed(inShouldBeSmoothed),
	parameterValueTree(inName)
{
//	DBGM("In ParameterValueTree::ParameterValueTree() ");
	constructValueTree();
	parameterValue.addListener(this);
	defaultValue.addListener(this);
	isSmoothed.addListener(this);
	
}

ParameterValueTree::~ParameterValueTree()
{
//	DBGM("In ParameterValueTree::~ParameterValueTree() ");
}

void ParameterValueTree::constructValueTree()
{
//	DBGM("In ParameterValueTree::constructValueTree() ");
	//parameter value, defaultvalue, isSmoothed
	
	// #TODO: CURRENT WORK SPOT, trying to fix valuetree for serialization
	parameterValueTree.setProperty("name", parameterName, nullptr);
	
	ValueTree valueChild("parameterValue");
	valueChild.setProperty("name", "Parameter Value", nullptr);
	valueChild.setProperty("value", parameterValue, nullptr);
	parameterValueTree.addChild(valueChild, -1, nullptr);

	ValueTree defaultValueChild("defaultParameterValue");
	defaultValueChild.setProperty("name", "Default Value", nullptr);
	defaultValueChild.setProperty("value", defaultValue.getValue(), nullptr);
	parameterValueTree.addChild(defaultValueChild, -1, nullptr);

	ValueTree parameterSmoothedChild("isSmoothed");
	parameterSmoothedChild.setProperty("name", "Is Smoothed", nullptr);
	parameterSmoothedChild.setProperty("value", isSmoothed.getValue(), nullptr);
	parameterValueTree.addChild(parameterSmoothedChild, -1, nullptr);

}

ValueTree ParameterValueTree::getValueTree()
{
//	DBGM("In ParameterValueTree::getValueTree() ");
	return parameterValueTree;
}

void ParameterValueTree::setFromInTree(ValueTree inTree)
{
//	DBGM("In ParameterValueTree::setFromInTree() ");
	//parameterName = inTree.getProperty("name");
	parameterName = inTree.getType().toString();
	DBG("Parameter Name: " + inTree.getType().toString());
	parameterValue = inTree.getChildWithName("parameterValue").getPropertyAsValue("value", nullptr).getValue();
	DBG("parameterValue: " + inTree.getChildWithName("parameterValue").getPropertyAsValue("value", nullptr).getValue().toString(););
	defaultValue = inTree.getChildWithName("defaultParameterValue").getPropertyAsValue("value", nullptr).getValue();
	DBG("defaultParameterValue: " + inTree.getChildWithName("defaultParameterValue").getPropertyAsValue("value", nullptr).getValue().toString());
	isSmoothed = inTree.getChildWithName("isSmoothed").getPropertyAsValue("value", nullptr).getValue();
	DBG("isSmoothed: " + inTree.getChildWithName("isSmoothed").getPropertyAsValue("value", nullptr).getValue().toString());
	parameterValueTree = inTree.createCopy();

}

void ParameterValueTree::valueChanged(Value& value)
{
//	DBGM("In ParameterValueTree::valueChanged() ");
	// #TODO: check to see if parameterValueTree is null!
	if (!parameterValueTree.isValid()) return;
	if(value.)
		parameterValueTree.getChildWithName("parameterValue").setProperty("value", value.getValue(), nullptr);
	else if (value == defaultValue)
		parameterValueTree.getChildWithName("defaultParameterValue").setProperty("value", value.getValue(), nullptr);
	else if (value == isSmoothed)
		parameterValueTree.getChildWithName("isSmoothed").setProperty("value", value.getValue(), nullptr);
}

}  // Namespace Zen