/* ==============================================================================
//	ZynVerbAudioProcessor.h
//	Part of the Zentropia JUCE Collection
//	@author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//	@version 0.1
//	@date 2015/09/08
//	Copyright (C) 2015 by Casey Bailey
//	Provided under the [GNU license]
//
//	Details: Main DSP Processing Class
//
//	Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/
#pragma warning(disable:4100)

#include "ZynVerbAudioProcessor.h"
#include "ZynVerbAudioProcessorEditor.h"
#include "zen_utils/utilities/ZenUtils.hpp"
#include "zen_utils/processing/BufferSampleProcesses.h"



//==============================================================================
ZynVerbAudioProcessor::ZynVerbAudioProcessor()
	//:rootTree("Root")
{
//	DBGM("In ZynVerbAudioProcessor::ZynVerbAudioProcessor() ");

//Visual Studio mem leak diagnostics settings 
#ifdef JUCE_MSVC
	_CrtSetDbgFlag(0);	//Turn off VS memory dump output
	//_crtBreakAlloc = 307;	//Break on this memory allocation number (When Debug)
#endif

	addParameter(audioGainParam = new DecibelParameter(
		"Gain", true, 0.01f, -96.0f, 12.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.01f, "dB"));
	addParameter(muteParam = new BooleanParameter("Mute", false));
	addParameter(bypassParam = new BooleanParameter("Bypass", false));

#ifdef ZEN_DEBUG
	rootTree = createParameterTree();
	debugWindow = ZenDebugEditor::getInstance();
	debugWindow->setSize(400, 400);
	//Open in bottom right corner
	debugWindow->setTopLeftPosition(1900 - debugWindow->getWidth(), 1040 - debugWindow->getHeight());
	debugWindow->setSource(rootTree);
#endif
}

ZynVerbAudioProcessor::~ZynVerbAudioProcessor()
{
//	DBGM("In ZynVerbAudioProcessor::~ZynVerbAudioProcessor() ");	
	audioGainParam = nullptr;
	muteParam = nullptr;
	bypassParam = nullptr;

	rootTree.removeAllChildren(nullptr);
	debugWindow = nullptr;
}

//==============================================================================
/**Note that if you have more outputs than inputs, then only those channels that 
correspond to an input channel are guaranteed to contain sensible data - 
e.g. in the case of 2 inputs and 4 outputs, the first two channels contain the input, 
but the last two channels may contain garbage, so you should be careful NOT to let 
this pass through without being overwritten or cleared.

ZenDebugUtils::timedPrint(String stringToPrint)*/
void ZynVerbAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	setCurrentSampleRate(getSampleRate());

#pragma region MIDI Example
	/* MIDI handling example
	MidiMessage mm;
	MidiBuffer processedMidi;
	int samplePos;

	for (MidiBuffer::Iterator mbi(midiMessages); mbi.getNextEvent(mm, samplePos); )
	{

		if (mm.isNoteOnOrOff())
		{
			//ZEN_LABEL_TRACE("MidiBuffer.isNoteOnOrOff", S(mm.isNoteOnOrOff()));
		} 
		if (mm.isNoteOn())
		{
			uint8 newVel = static_cast<uint8>(mm.getVelocity());

			// Changing output buffer
			mm = MidiMessage::noteOn(mm.getChannel(), mm.getNoteNumber(), newVel);
		} else if (mm.isNoteOff())
		{
			ZEN_REMOVE_LABEL_TRACE("MidiBuffer.getNoteNumber(" + S(noteNumber) + "):");
		} else if (mm.isAftertouch())
		{
		} else if (mm.isPitchWheel())
		{
		}
		//add the newly calculated event to new buffer
		processedMidi.addEvent(mm, samplePos);
	}
	//put the new midi msg buffer onto the buffer to be exported
	midiMessages.swapWith(processedMidi);
	*/
#pragma endregion

	jassert(currentSampleRate >= 0);

	if (bypassParam->isOn()) return;

	float* leftData = buffer.getWritePointer(0);	//leftData references left channel now
	float* rightData = buffer.getWritePointer(1); //right data references right channel now

	//Audio buffer visualization
	ZEN_DEBUG_BUFFER("Left Buffer Pre", leftData, buffer.getNumSamples(), -1, 1);
	ZEN_DEBUG_BUFFER("Right Buffer Pre", rightData, buffer.getNumSamples(), -1, 1);

	if (muteParam->isOn())
	{
		buffer.applyGain(0.0f);
		return;
	}

	//Main Processing Loop
	for (long i = 0; i < buffer.getNumSamples(); i++)
	{
		//Make sure screwups don't blow up speakers
		float audioGainRaw = getClamped(audioGainParam->getSmoothedRawDecibelGainValue(), 0, 4.0f); 
		jassert(audioGainRaw >= 0);
		ZEN_LABEL_TRACE("audioGainRaw", S(audioGainRaw));
		ZEN_LABEL_TRACE("Left", S(leftData[i]));
		ZEN_LABEL_TRACE("Right", S(rightData[i]));

		BufferSampleProcesses::processGain(&leftData[i], &rightData[i], audioGainRaw);
	}

	//Audio buffer visualization 

	ZEN_DEBUG_BUFFER("Left Buffer Post", leftData, buffer.getNumSamples(), -1, 1);
	ZEN_DEBUG_BUFFER("Right Buffer Post", rightData, buffer.getNumSamples(), -1, 1);

}

// You should use this method to store your parameterSet in the memory block.
void ZynVerbAudioProcessor::getStateInformation(MemoryBlock& destData)
{
//	DBGM("In ZynVerbAudioProcessor::getStateInformation() ");

	XmlElement rootXML("Root");

	for (auto param : getParameters())
	{
		ZenParameter* zenParam = dynamic_cast<ZenParameter*>(param);
		zenParam->writeToXML(rootXML);
	}
	//DBG(rootXML.createDocument("", false, false, "UTF-8", 120));
	copyXmlToBinary(rootXML, destData);
}

// You should use this method to restore your parameters from this memory block,
// whose contents will have been created by the getStateInformation() call.
void ZynVerbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
//	DBGM("In ZynVerbAudioProcessor::setStateInformation() ");

	ScopedPointer<XmlElement> theXML = this->getXmlFromBinary(data, sizeInBytes);
	//DBG(theXML->createDocument("", false, false, "UTF-8", 120));

	if (theXML != nullptr)
	{
		for (auto param : getParameters())
		{
			ZenParameter* zenParam = dynamic_cast<ZenParameter*>(param);
			if(zenParam != nullptr) zenParam->setFromXML(*theXML);
		}
	}
}

ValueTree ZynVerbAudioProcessor::createParameterTree()
{
	ValueTree valTree("Parameters");

	for (auto param : getParameters())
	{
		ZenParameter* zenParam = dynamic_cast<ZenParameter*>(param);
		if(zenParam != nullptr) valTree.addChild(zenParam->getValueTree(), -1, nullptr);
	}
	//DBG("Value Tree result from createParameterTree() : " + valTree.toXmlString());
	return valTree;
}

#pragma region overrides
//==============================================================================
void ZynVerbAudioProcessor::prepareToPlay(double inSampleRate, int samplesPerBlock)
{
//	DBGM("In ZynVerbAudioProcessor::prepareToPlay() ");
	// Use this method as the place to do any pre-playback
	// initialisation that you need..

	// Iterates over parameters and resets Smooth for the ones who need it
	for (auto param : getParameters())
	{
		ZenParameter* zenParam = dynamic_cast<ZenParameter*>(param);
		if (zenParam != nullptr)
		{
			if (zenParam->checkShouldBeSmoothed())
			{
				zenParam->resetSmoothedValue(inSampleRate);
			}
		}
	}
}

//==============================================================================
const String ZynVerbAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

const String ZynVerbAudioProcessor::getInputChannelName(int channelIndex) const
{
	return String(channelIndex + 1);
}


const String ZynVerbAudioProcessor::getOutputChannelName(int channelIndex) const
{
	return String(channelIndex + 1);
}

bool ZynVerbAudioProcessor::isInputChannelStereoPair(int index) const
{
	return true;
}

bool ZynVerbAudioProcessor::isOutputChannelStereoPair(int index) const
{
	return true;
}

bool ZynVerbAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool ZynVerbAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool ZynVerbAudioProcessor::silenceInProducesSilenceOut() const
{
	return false;
}

double ZynVerbAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int ZynVerbAudioProcessor::getNumPrograms()
{
	return 1;	// NB: some hosts don't cope very well if you tell them there are 0 programs,
		// so this should be at least 1, even if you're not really implementing programs.
}

int ZynVerbAudioProcessor::getCurrentProgram()
{
	return 0;
}

void ZynVerbAudioProcessor::setCurrentProgram(int index)
{
}

const String ZynVerbAudioProcessor::getProgramName(int index)
{
	return String();
}

void ZynVerbAudioProcessor::changeProgramName(int index, const String& newName)
{
}

void ZynVerbAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}



//==============================================================================
bool ZynVerbAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ZynVerbAudioProcessor::createEditor()
{
//	DBGM("In ZynVerbAudioProcessor::createEditor() ");
	return new ZynVerbAudioProcessorEditor(*this);
}

#pragma endregion
//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
//	DBGM("In ZynVerbAudioProcessor::createPluginFilter() ");
	return new ZynVerbAudioProcessor();
}

