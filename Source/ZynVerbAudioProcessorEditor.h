/*==============================================================================
//  ZynVerbAudioProcessorEditor.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/14
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Main GUI Component Controller
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#ifndef __JUCE_HEADER_E0F368D2F71ED5FC__
#define __JUCE_HEADER_E0F368D2F71ED5FC__

#include "JuceHeader.h"
#include "ZynVerbAudioProcessor.h"
#include "zen_utils/components/ZenComponents.h"

using namespace Zen;

/// <summary> GUI Editor for zynth audio processor. </summary>
class ZynVerbAudioProcessorEditor  : public AudioProcessorEditor,
                                   public Timer,
	                               public DragAndDropContainer,
                                   public ButtonListener,
                                   public SliderListener
{
public:
    //==============================================================================
	explicit ZynVerbAudioProcessorEditor (ZynVerbAudioProcessor& ownerFilter);
    ~ZynVerbAudioProcessorEditor();

    
	void timerCallback() override;    

	//==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;

private:
	ZynVerbAudioProcessor* processor;
    
    //==============================================================================
	ScopedPointer<TabbedComponent> mainTabsComponent;
    ScopedPointer<AssociatedTextButton> muteButton;
    ScopedPointer<AssociatedSlider> gainSlider;
    ScopedPointer<AssociatedTextButton> bypassButton;
	
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZynVerbAudioProcessorEditor)
	
};

#endif   // __JUCE_HEADER_E0F368D2F71ED5FC__