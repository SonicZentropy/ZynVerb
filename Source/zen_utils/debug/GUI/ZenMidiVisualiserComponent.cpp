/*==============================================================================
//  ZenMidiVisualiserComponent.cpp
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/29
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: //  Details: Pressed MIDI keys visualiser

//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#include "ZenMidiVisualiserComponent.h"

namespace Zen
{

ZenMidiVisualiserComponent::ZenMidiVisualiserComponent(const String& midiVisualiserName)
	: 
	lastInputIndex(0), isAddingFromMidiInput(false),
	keyboardComponent(keyboardState, MidiKeyboardComponent::horizontalKeyboard),
	midiLogListBoxModel(midiMessageList)
{
	this->setName(midiVisualiserName);
	deviceManager = new AudioDeviceManager();
	deviceManager->initialiseWithDefaultDevices(2, 2);
	
	setOpaque(true);

	addAndMakeVisible(midiInputListLabel);
	midiInputListLabel.setText("MIDI Input:", dontSendNotification);
	midiInputListLabel.attachToComponent(&midiInputList, true);

	midiInputList.setBounds(10, 10, 450, 300);

	addAndMakeVisible(midiInputList);
	midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
	const StringArray midiInputs(MidiInput::getDevices());
	midiInputList.addItemList(midiInputs, 1);
	midiInputList.addListener(this);

	// find the first enabled device and use that by default
	for (int i = 0; i < midiInputs.size(); ++i)
	{
		if (deviceManager->isMidiInputEnabled(midiInputs[i]))
		{
			setMidiInput(i);
			break;
		}
	}

	// if no enabled devices were found just use the first one in the list
	if (midiInputList.getSelectedId() == 0)
		setMidiInput(0);

	addAndMakeVisible(keyboardComponent);
	keyboardState.addListener(this);

	addAndMakeVisible(messageListBox);
	messageListBox.setModel(&midiLogListBoxModel);
	messageListBox.setColour(ListBox::backgroundColourId, Colour(0x32ffffff));
	messageListBox.setColour(ListBox::outlineColourId, Colours::black);
}

ZenMidiVisualiserComponent::~ZenMidiVisualiserComponent()
{
	keyboardState.removeListener(this);
	deviceManager->removeMidiInputCallback(MidiInput::getDevices()[midiInputList.getSelectedItemIndex()], this);
	midiInputList.removeListener(this);
}

String ZenMidiVisualiserComponent::getMidiMessageDescription(const MidiMessage& m)
{
	if (m.isNoteOn())           return "Note on: " + S(m.getNoteNumber()) + " (" + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ") Vel: " + S(m.getVelocity());
	if (m.isNoteOff())          return "Note off: " + S(m.getNoteNumber()) + " (" + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ")";
	if (m.isProgramChange())    return "Program change (Number): " + String(m.getProgramChangeNumber());
	if (m.isPitchWheel())       return "Pitch wheel: " + String(m.getPitchWheelValue());
	if (m.isAftertouch())       return "After touch: " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + String(m.getAfterTouchValue());
	if (m.isChannelPressure())  return "Channel pressure: " + String(m.getChannelPressureValue());
	if (m.isAllNotesOff())      return "All notes off";
	if (m.isAllSoundOff())      return "All sound off";
	if (m.isMetaEvent())        return "Meta event";

	if (m.isController())
	{
		String temp = MidiMessage::getControllerName(m.getControllerNumber());
		String name = "Controller [" + S(m.getControllerNumber()) + "]";
		if (!temp.isEmpty())
			name += " " + temp;

		return name + ": " + String(m.getControllerValue());
	}

	return String::toHexString(m.getRawData(), m.getRawDataSize());
}

//==============================================================================
MidiLogListBoxModel::MidiLogListBoxModel(const Array<MidiMessage>& list) : midiMessageList(list)
{

}

int MidiLogListBoxModel::getNumRows()
{
	return midiMessageList.size();
}



void MidiLogListBoxModel::paintListBoxItem(int row, Graphics& g, int width, int height, bool rowIsSelected)
{
	if (rowIsSelected)
		g.fillAll(Colours::blue.withAlpha(0.2f));

	if (isPositiveAndBelow(row, midiMessageList.size()))
	{
		g.setColour(Colours::black);

		const MidiMessage& message = midiMessageList.getReference(row);
		double time = message.getTimeStamp();

		g.drawText(String::formatted("%02d:%02d:%02d",
			static_cast<int>(time / 3600.0) % 24,
			static_cast<int>(time / 60.0) % 60,
			static_cast<int>(time) % 60)
			+ " - " + ZenMidiVisualiserComponent::getMidiMessageDescription(message),
			Rectangle<int>(width, height).reduced(4, 0),
			Justification::centredLeft, true);
	}
}

//==============================================================================





void ZenMidiVisualiserComponent::handleAsyncUpdate()
{
	messageListBox.updateContent();
	messageListBox.scrollToEnsureRowIsOnscreen(midiMessageList.size() - 1);
	messageListBox.repaint();
}

void ZenMidiVisualiserComponent::addMessageToList(const MidiMessage& message)
{
	midiMessageList.add(message);
	triggerAsyncUpdate();
}

void ZenMidiVisualiserComponent::postMessageToList(const MidiMessage& message)
{
	(new IncomingMessageCallback(this, message))->post();
}

void ZenMidiVisualiserComponent::IncomingMessageCallback::messageCallback()
{
	if (demo != nullptr)
		demo->addMessageToList(message);
}

ZenMidiVisualiserComponent::IncomingMessageCallback::IncomingMessageCallback(ZenMidiVisualiserComponent* d, const MidiMessage& m) : demo(d), message(m)
{

}

void ZenMidiVisualiserComponent::handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber)
{
	if (!isAddingFromMidiInput)
	{
		MidiMessage m(MidiMessage::noteOff(midiChannel, midiNoteNumber));
		m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
		postMessageToList(m);
	}
}

void ZenMidiVisualiserComponent::handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
	if (!isAddingFromMidiInput)
	{
		MidiMessage m(MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity));
		m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
		postMessageToList(m);
	}
}

void ZenMidiVisualiserComponent::handleIncomingMidiMessage(MidiInput*, const MidiMessage& message)
{
	// ReSharper disable once CppLocalVariableWithNonTrivialDtorIsNeverUsed
	const ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
	keyboardState.processNextMidiEvent(message);
	postMessageToList(message);
}

void ZenMidiVisualiserComponent::comboBoxChanged(ComboBox* box)
{
	if (box == &midiInputList)
		setMidiInput(midiInputList.getSelectedItemIndex());
}

void ZenMidiVisualiserComponent::setMidiInput(int index)
{
	const StringArray list(MidiInput::getDevices());

	deviceManager->removeMidiInputCallback(list[lastInputIndex], this);

	const String newInput(list[index]);

	if (!deviceManager->isMidiInputEnabled(newInput))
		deviceManager->setMidiInputEnabled(newInput, true);

	deviceManager->addMidiInputCallback(newInput, this);
	midiInputList.setSelectedId(index + 1, dontSendNotification);

	lastInputIndex = index;
}

void ZenMidiVisualiserComponent::resized()
{
	Rectangle<int> area(getLocalBounds());
	midiInputList.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(8));
	keyboardComponent.setBounds(area.removeFromTop(80).reduced(8));
	messageListBox.setBounds(area.reduced(8));
}

void ZenMidiVisualiserComponent::paint(Graphics& g)
{
	g.setColour (Colour::greyLevel (0.2f)); 
	g.fillAll(); 
}


} // Namespace Zen