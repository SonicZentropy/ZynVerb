/*==============================================================================
//  ZynVerbAudioProcessorEditor.cpp
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/26
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Main GUI Component class
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/


#include "ZynVerbAudioProcessorEditor.h"
#include "zen_utils\debug\ZenDebugEditor.h"

//==============================================================================
ZynVerbAudioProcessorEditor::ZynVerbAudioProcessorEditor(ZynVerbAudioProcessor& ownerFilter)
	: AudioProcessorEditor(ownerFilter)
{
//	DBGM("In ZynVerbAudioProcessorEditor::ZynVerbAudioProcessorEditor() ");
	processor = &ownerFilter;
	setName("ZynVerbMainComponent");

	mainTabsComponent = new TabbedComponent(TabbedButtonBar::TabsAtTop);
	mainTabsComponent->setName("Main Tabs");
	mainTabsComponent->addTab("Utility", Colours::darkgrey, new Component("Utility"), true, 0);
	addAndMakeVisible(mainTabsComponent);

	mainTabsComponent->getTabContentComponent(0)->addAndMakeVisible (
		muteButton = new AssociatedTextButton("Mute Button", processor->muteParam));
    muteButton->setTooltip ("Mute all audio");
    muteButton->setButtonText ("MUTE");
	muteButton->setClickingTogglesState(true);
    muteButton->addListener (this);

	
	mainTabsComponent->getTabContentComponent(0)->addAndMakeVisible (
		gainSlider = new AssociatedSlider ("Gain Slider", processor->audioGainParam, "dB"));
    gainSlider->setTooltip ("Adjusts audio gain");
    gainSlider->setRange (-96, 12, 0.01);
    gainSlider->setSliderStyle (Slider::LinearHorizontal);
    gainSlider->setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
	gainSlider->setTextValueSuffix("dB");
	gainSlider->setDoubleClickReturnValue(true, 0.0);
	gainSlider->addListener (this);
	
	mainTabsComponent->getTabContentComponent(0)->addAndMakeVisible (
		bypassButton = new AssociatedTextButton ("Bypass Button", processor->bypassParam));
    bypassButton->setTooltip ("Bypass Plugin");
    bypassButton->setButtonText ("Bypass");
	bypassButton->setClickingTogglesState(true);
    bypassButton->addListener (this);
	
	ZEN_COMPONENT_DEBUG_ATTACH(this);

	this->setSize(600, 600);
	startTimer(50); // Start timer poll with 50ms rate
}

ZynVerbAudioProcessorEditor::~ZynVerbAudioProcessorEditor()
{
//	DBGM("In ZynVerbAudioProcessorEditor::~ZynVerbAudioProcessorEditor() ");
	
	mainTabsComponent = nullptr;
    muteButton = nullptr;
    gainSlider = nullptr;
    bypassButton = nullptr;		
	
//#ifdef ZEN_DEBUG
	ZenDebugEditor::removeComponentDebugger();
//#endif // ZEN_DEBUG
}

//==============================================================================
void ZynVerbAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colour (0xff303030));
}

void ZynVerbAudioProcessorEditor::resized()
{
//	DBGM("In ZynVerbAudioProcessorEditor::resized() 
	mainTabsComponent->setBounds(0, 0, getWidth(), getHeight());
    muteButton->setBounds (10, 6, 74, 24);
    gainSlider->setBounds (158, 8, 150, 24);
    bypassButton->setBounds (10, 38, 74, 24);
}


void ZynVerbAudioProcessorEditor::buttonClicked(Button* buttonThatWasClicked)
{	
//	 DBGM("In ZynVerbAudioProcessorEditor::buttonClicked() ");
	 dynamic_cast<AssociatedButton*>(buttonThatWasClicked)->setAssociatedParameterValueNotifyingHost();
}

void ZynVerbAudioProcessorEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{    		
//	DBGM("In ZynVerbAudioProcessorEditor::sliderValueChanged()");
	dynamic_cast<AssociatedSlider*>(sliderThatWasMoved)->setAssociatedParameterValueNotifyingHost();
}


void ZynVerbAudioProcessorEditor::timerCallback()
{
	//undoManager.beginNewTransaction();
	AssociatedSlider* currentSlider;
	AssociatedButton* currentButton;
	AssociatedComponent* currentComponent;
	ZenParameter* currentParameter;

	int numComponents = this->getNumChildComponents();
	
	//This looks ugly, but it's a whole lot better than a massive if chain checking EVERY param individually
	//This should handle every component's GUI updates automatically
	for (int i = 0; i < numComponents; i++)
	{
		currentComponent = dynamic_cast<AssociatedComponent*>(this->getChildComponent(i));
		if (nullptr == currentComponent) continue;

		currentParameter = currentComponent->getAssociatedParameter();
		if (!currentParameter->needsUIUpdate()) //Don't need to keep going if the Component doesn't need to be updated
		{
			continue;
		}

		currentSlider = dynamic_cast<AssociatedSlider*>(currentComponent);
		if(currentSlider != nullptr)
		{
			currentSlider->setValue(currentParameter->getValueForGUIComponent(), dontSendNotification);
 		} 
		else 
		{
			currentButton = dynamic_cast<AssociatedButton*>(this->getChildComponent(i));
			if (currentButton != nullptr)
			{
				currentButton->setToggleState(currentParameter->getBoolFromValue(), dontSendNotification);
 			}
			else
			{
				DBG("In ZynVerbAudioProcessorEditor::timerCallback() ");
				jassertfalse;
			}
		}
		currentParameter->resetUIUpdate(); //Finished Updating UI
	}
}

