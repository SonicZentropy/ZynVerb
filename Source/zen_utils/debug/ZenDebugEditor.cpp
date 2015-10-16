/*==============================================================================
//  ZenDebugEditor.cpp
//  Part of the Zentropia JUCE Collection
//  @author Casey Bailey (<a href="SonicZentropy@gmail.com">email</a>)
//  @version 0.1
//  @date 2015/09/28
//  Copyright (C) 2015 by Casey Bailey
//  Provided under the [GNU license]
//
//  Details: Implementation for ZenDebugEditor.h
//
//  Zentropia is hosted on Github at [https://github.com/SonicZentropy]
===============================================================================*/

#include "ZenDebugEditor.h"

namespace Zen
{
	ZenDebugEditor::ZenDebugEditor() :
		DocumentWindow("Zen Debug",
			Colours::lightgrey,
			DocumentWindow::allButtons)
	{
		this->setName("ValueTreeEditorWindow");
	
		tabsComponent = new TabbedComponent(TabbedButtonBar::TabsAtTop);
		tabsComponent->setName("DebugTabbedComponent");
		
		valueTreeEditorComponent = new ValueTreeEditor::Editor("ValueTreeEditor");
		tabsComponent->addTab("Params", Colours::lightgrey, valueTreeEditorComponent, false);

		bufferVisualiserComponent = new BufferVisualiser("BufferVisualiser");
		tabsComponent->addTab("Buffers", Colours::lightgrey, bufferVisualiserComponent->getComponent(), false);

		midiVisualiserComponent = new ZenMidiVisualiserComponent("MidiVisualiser");
		tabsComponent->addTab("MIDI", Colours::lightgrey, midiVisualiserComponent, false);

		notepadComponent = new NotepadComponent("Notepad", "");
		tabsComponent->addTab("Notes", Colours::lightgrey, notepadComponent, false);

		componentVisualiserComponent = nullptr;

		this->setSize(400, 400);
		tabsComponent->setCurrentTabIndex(0);
		tabsComponent->setBounds(0, 0, getWidth(), getHeight());

		setContentNonOwned(tabsComponent, false);
		setResizable(true, false);
		setUsingNativeTitleBar(true);
		centreWithSize(getWidth(), getHeight());
		setVisible(true);

	}

	ZenDebugEditor::~ZenDebugEditor()
	{
		valueTreeEditorComponent->setTree(ValueTree::invalid);
		if (valueTreeEditorComponent->isOnDesktop())
		{
			valueTreeEditorComponent->removeFromDesktop();
		}
		valueTreeEditorComponent = nullptr;
		if (this->isOnDesktop())
		{
			this->removeFromDesktop();
		}
		tabsComponent = nullptr;
		bufferVisualiserComponent = nullptr;
		componentVisualiserComponent = nullptr;

		clearSingletonInstance();
	}

	void ZenDebugEditor::addTraceLabel(const String& inName, const String& inText)
	{
		this->valueTreeEditorComponent->addTraceLabel(inName, inText);
	}

	void ZenDebugEditor::addTraceLabel(const String& inName, Value& theValue)
	{
		this->valueTreeEditorComponent->addTraceLabel(inName, theValue);
	}

	void ZenDebugEditor::removeTraceLabel(const String& inName)
	{
		this->valueTreeEditorComponent->removeTraceLabel(inName);
	}

	void ZenDebugEditor::setLabelText(const String& labelName, const String& inText)
	{
		this->valueTreeEditorComponent->setLabelText(labelName, String(inText));
	}

	void ZenDebugEditor::setLabelText(const String& labelName, const float inText)
	{
		this->valueTreeEditorComponent->setLabelText(labelName, inText);
	}

	void ZenDebugEditor::addOrSetTraceLabel(const String& inName, const String& inText)
	{
		this->valueTreeEditorComponent->addOrSetTraceLabel(inName, inText);
	}

	void ZenDebugEditor::closeButtonPressed()
	{
		setVisible(false);
	}

	void ZenDebugEditor::setSource(ValueTree& v)
	{
		valueTreeEditorComponent->setTree(v);
	}

	void ZenDebugEditor::attachComponentDebugger(Component* rootComponent)
	{
		componentVisualiserComponent = new ComponentDebugger(rootComponent, getWidth(), getHeight(), "ComponentDebugger");
		
		tabsComponent->addTab("Comps", Colours::lightgrey, componentVisualiserComponent, false, 3);
		//refreshComponentDebugger();
	}

	void ZenDebugEditor::removeInstanceComponentDebugger()
	{
		tabsComponent->removeTab(tabsComponent->getTabIndexByName("ComponentDebugger"));
		componentVisualiserComponent = nullptr;
	}

	void ZenDebugEditor::refreshComponentDebugger()
	{
		int tabCompIndex = tabsComponent->getTabIndexByName("Comps");
		ComponentDebugger* compTab = dynamic_cast<ComponentDebugger*>(tabsComponent->getTabContentComponent(tabCompIndex));
		if (compTab != nullptr)
			compTab->refresh();
	}

	void ZenDebugEditor::resized()
	{
		//DBGM("In ZenDebugEditor::resized() ");
		tabsComponent->setSize(getWidth(), getHeight());
		for (auto currComponent : tabsComponent->getTabContentComponentArray())
		{
			currComponent->setSize(getWidth(), getHeight());
		}
	}

	void ZenDebugEditor::removeComponentDebugger()
	{
		if (_singletonInstance != nullptr)
		{			
			if (_singletonInstance->tabsComponent->getCurrentTabName() == "Comps")
				_singletonInstance->tabsComponent->setCurrentTabIndex(0);
			_singletonInstance->removeInstanceComponentDebugger();			
		}
	}

	juce_ImplementSingleton(ZenDebugEditor)

	

} // namespace Zen

