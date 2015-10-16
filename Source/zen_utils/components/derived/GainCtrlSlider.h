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

#ifndef GAINCTRLSLIDER_H_INCLUDED
#define GAINCTRLSLIDER_H_INCLUDED

#include "JuceHeader.h"
#include "../AssociatedSlider.h"

namespace Zen
{
	class GainCtrlSlider : public AssociatedSlider
	{
	public:

		GainCtrlSlider(const String& componentName, ZenParameter* associatedParam);
		GainCtrlSlider(const String& componentName, ZenParameter* associatedParam, const String& desiredUnitLabel);
		GainCtrlSlider(const String& componentName, ZenParameter* associatedParam, const String& desiredUnitLabel, const float& desiredMaxDBRange);

		~GainCtrlSlider();

		String getTextFromValue(double value) override;
		double getValueFromText(const String& text) override;

		float getMaximumDecibelsInRange() const { return maximumDecibelsInRange; }
		void setMaximumDecibelsInRange(float val) { maximumDecibelsInRange = val; }

	private:

		float maximumDecibelsInRange = 0;

	};
}
#endif  // GAINCTRLSLIDER_H_INCLUDED