/*==============================================================================
//  ComponentValueTree.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/21
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: ValueTree representation of a Component
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#ifndef COMPONENTVALUETREE_H_INCLUDED
#define COMPONENTVALUETREE_H_INCLUDED
#include "JuceHeader.h"

#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

namespace Zen{

class ComponentValueTree : public ComponentListener
{

public:
	
	ComponentValueTree();
	~ComponentValueTree();

private:

};
} // namespace Zen
#endif // COMPONENTVALUETREE_H_INCLUDED
