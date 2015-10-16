/*==============================================================================
//  ZenUtils.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/18
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: General utility functions in anon namespace
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/


#ifndef ZENUTILS_H_INCLUDED
#define ZENUTILS_H_INCLUDED



namespace
{

/// <summary>Return value clamped between min/max</summary>
inline float getClamped(const float& valueToClamp, const float& lowerClampBound, const float& upperClampBound)
{
	return (valueToClamp > upperClampBound) ? upperClampBound : ((valueToClamp < lowerClampBound) ? lowerClampBound : valueToClamp);
}

} // namespace
#endif // ZENUTILS_H_INCLUDED