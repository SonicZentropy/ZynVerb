/*==============================================================================
//  ParameterValueTree.h
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

#ifndef PARAMETERVALUETREE_H_INCLUDED
#define PARAMETERVALUETREE_H_INCLUDED
#include "JuceHeader.h"

namespace Zen{

class ParameterValueTree : public ValueListener
{
public:
	
	ParameterValueTree();
	ParameterValueTree(String inName, Value parameterValueReference, float inDefaultValue, bool inShouldBeSmoothed);

	~ParameterValueTree();


	void constructValueTree();

	ValueTree getValueTree();

	void setFromInTree(ValueTree inTree);
	virtual void valueChanged(Value& value) override;
protected:
	ValueTree parameterValueTree;


private:

};

} // Namespace Zen

#endif // PARAMETERVALUETREE_H_INCLUDED
