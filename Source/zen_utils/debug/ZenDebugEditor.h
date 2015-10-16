/*==============================================================================
//  ZenDebugEditor.h
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/28
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Zen Debug Component for window - contains Value Tree Parameter tab,
//  Buffer Visualization Tab, and Labels tab
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/
#ifndef ZENDEBUGCOMPONENT_H_INCLUDED
#define ZENDEBUGCOMPONENT_H_INCLUDED
#include "JuceHeader.h"
#include "GUI/value_tree_editor.h"

namespace Zen
{
	class ZenDebugEditor : public DocumentWindow
	{
	public:

		ZenDebugEditor();
		~ZenDebugEditor();

		void addTraceLabel(const String& inName, const String& inText);
		void addTraceLabel(const String& inName, Value& theValue);

		void removeTraceLabel(const String& inName);

		void setLabelText(const String& labelName, const String& inText);
		void setLabelText(const String& labelName, const float inText);

		void addOrSetTraceLabel(const String& inName, const String& inText);

		void closeButtonPressed() override;

		void setSource(ValueTree& v);

		void attachComponentDebugger(Component* rootComponent);

		void removeInstanceComponentDebugger();

		void refreshComponentDebugger();
		void resized() override;

		juce_DeclareSingleton(ZenDebugEditor, false);

		static void removeComponentDebugger();
	private:
	
		ScopedPointer<TabbedComponent> tabsComponent;
		ScopedPointer<ValueTreeEditor::Editor> valueTreeEditorComponent;
		ScopedPointer<BufferVisualiser> bufferVisualiserComponent;
		ScopedPointer<ZenMidiVisualiserComponent> midiVisualiserComponent;
		ScopedPointer<ComponentDebugger> componentVisualiserComponent;
		ScopedPointer<NotepadComponent> notepadComponent;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZenDebugEditor);
	};

	//==============================================================================
	// INLINE MACRO FUNCTION TRANSLATIONS
	//==============================================================================

	#ifdef ZEN_DEBUG
	inline void ZEN_LABEL_TRACE(const String& labelName, const String& labelText)
	{
		ZenDebugEditor::getInstance()->addOrSetTraceLabel(labelName, labelText);
	};

	inline void ZEN_REMOVE_LABEL_TRACE(const String& labelName)
	{
		ZenDebugEditor::getInstance()->removeTraceLabel(labelName);
	}

	inline void ZEN_DEBUG_BUFFER(const String & name, float * data, int size, float min, float max)
	{
		Store::getInstance()->record(name, data, size, min, max);
	}


	inline void ZEN_COMPONENT_DEBUG_ATTACH(Component* rootComponent)
	{
		ZenDebugEditor::getInstance()->attachComponentDebugger(rootComponent);
	}

	#else
	inline void ZEN_LABEL_TRACE(const String& labelName, const String& labelText)
	{};

	inline void ZEN_REMOVE_LABEL_TRACE(const String& labelName)
	{};

	inline void ZEN_DEBUG_BUFFER(const String & name, float * data, int size, float min, float max)
	{};

	inline void ZEN_COMPONENT_DEBUG_ATTACH(Component* rootComponent)
	{};


	#endif // ZEN_DEBUG
} // Namespace Zen
#endif // ZENDEBUGCOMPONENT_H_INCLUDED
