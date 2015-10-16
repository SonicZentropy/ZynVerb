/*==============================================================================
//  ZenMidiVisualiserComponent.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/29
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Pressed MIDI keys visualiser
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#ifndef ZENMIDIVISUALISER_H_INCLUDED
#define ZENMIDIVISUALISER_H_INCLUDED
#include "JuceHeader.h"

namespace Zen{

//==============================================================================
/** Simple list box that just displays a StringArray. */
class MidiLogListBoxModel : public ListBoxModel
{
public:
	explicit MidiLogListBoxModel(const Array<MidiMessage>& list);

	int getNumRows() override;

	void paintListBoxItem(int row, Graphics& g, int width, int height, bool rowIsSelected) override;

private:
	const Array<MidiMessage>& midiMessageList;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLogListBoxModel)
};

//==============================================================================
class ZenMidiVisualiserComponent : public Component,
	private ComboBox::Listener,
	private MidiInputCallback,
	private MidiKeyboardStateListener,
	private AsyncUpdater
{
public:
	explicit ZenMidiVisualiserComponent(const String& midiVisualiserName = "MidiVisualiserComponent");
	~ZenMidiVisualiserComponent();

	void paint(Graphics& g) override;

	void resized() override;

	static String getMidiMessageDescription(const MidiMessage& m);

private:
	ScopedPointer<AudioDeviceManager> deviceManager;
	ComboBox midiInputList;
	Label midiInputListLabel;
	int lastInputIndex;
	bool isAddingFromMidiInput;

	MidiKeyboardState keyboardState;
	MidiKeyboardComponent keyboardComponent;

	ListBox messageListBox;
	Array<MidiMessage> midiMessageList;
	MidiLogListBoxModel midiLogListBoxModel;

	//==============================================================================
	/** Starts listening to a MIDI input device, enabling it if necessary. */
	void setMidiInput(int index);

	void comboBoxChanged(ComboBox* box) override;

	// These methods handle callbacks from the midi device + on-screen keyboard..
	void handleIncomingMidiMessage(MidiInput*, const MidiMessage& message) override;

	void handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

	void handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber) override;

	// This is used to dispach an incoming message to the message thread
	struct IncomingMessageCallback : public CallbackMessage
	{
		IncomingMessageCallback(ZenMidiVisualiserComponent* d, const MidiMessage& m);

		void messageCallback() override;

		Component::SafePointer<ZenMidiVisualiserComponent> demo;
		MidiMessage message;
	};

	void postMessageToList(const MidiMessage& message);

	void addMessageToList(const MidiMessage& message);

	void handleAsyncUpdate() override;

	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZenMidiVisualiserComponent);
};




} // namespace Zen
#endif // ZENMIDIVISUALISER_H_INCLUDED
