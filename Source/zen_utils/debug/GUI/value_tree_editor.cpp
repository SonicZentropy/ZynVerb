

#include "value_tree_editor.h"


#include <sstream>


namespace Zen
{

ValueTreeEditor::ValueTreeEditor(int componentWidth, int componentHeight) :
	DocumentWindow("Value Tree Editor",
		Colours::lightgrey,
		DocumentWindow::allButtons)
{
	

	this->setName("ValueTreeEditorWindow");
	tabsComponent = new TabbedComponent(TabbedButtonBar::TabsAtTop);
	tabsComponent->setName("DebugTabbedComponent");

	valueTreeEditorComponent = new Editor("ValueTreeEditor");
	tabsComponent->addTab("Params", Colours::lightgrey, valueTreeEditorComponent, false);

	bufferVisualiserComponent = new BufferVisualiser("BufferVisualiser");
	tabsComponent->addTab("Buffers", Colours::lightgrey, bufferVisualiserComponent->getComponent(), false);

	midiVisualiserComponent = new ZenMidiVisualiserComponent("MidiVisualiser");
	tabsComponent->addTab("MIDI", Colours::lightgrey, midiVisualiserComponent, false);

	notepadComponent = new NotepadComponent("Notepad", "");
	tabsComponent->addTab("Notes", Colours::lightgrey, notepadComponent, false);
	
	componentVisualiserComponent = nullptr;

	tabsComponent->setCurrentTabIndex(0);
	tabsComponent->setBounds(0, 0, componentWidth, componentHeight);
	
	setContentNonOwned(tabsComponent, true);
	setResizable(true, false);
	setUsingNativeTitleBar(true);
	centreWithSize(getWidth(), getHeight());
	setVisible(true);

	this->setSize(componentWidth, componentHeight);

}


ValueTreeEditor::~ValueTreeEditor()
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
}

/*
void ValueTreeEditor::addTraceLabel(const String& inName, Value& theValue)
{
	this->valueTreeEditorComponent->addTraceLabel(inName, theValue);
}

void ValueTreeEditor::addTraceLabel(const String& inName, const String& inText)
{
	this->valueTreeEditorComponent->addTraceLabel(inName, inText);
}

void ValueTreeEditor::removeTraceLabel(const String& inName)
{
	this->valueTreeEditorComponent->removeTraceLabel(inName);
}

void ValueTreeEditor::setLabelText(const String& labelName, const float inText)
{
	this->valueTreeEditorComponent->setLabelText(labelName, String(inText));
}

void ValueTreeEditor::setLabelText(const String& labelName, const String& inText)
{
	this->valueTreeEditorComponent->setLabelText(labelName, inText);
}

void ValueTreeEditor::addOrSetTraceLabel(const String& inName, const String& inText)
{
	this->valueTreeEditorComponent->addOrSetTraceLabel(inName, inText);
}

void ValueTreeEditor::closeButtonPressed()
{
	setVisible(false);
}

void ValueTreeEditor::setSource(ValueTree& v)
{
	valueTreeEditorComponent->setTree(v);
}

void ValueTreeEditor::attachComponentDebugger(Component* rootComponent)
{
		componentVisualiserComponent = new ComponentDebugger(rootComponent, getWidth(), getHeight(), "ComponentDebugger");
		tabsComponent->addTab("Comps", Colours::lightgrey, componentVisualiserComponent, false, 3);
}

void ValueTreeEditor::removeComponentDebugger()
{
	tabsComponent->removeTab( tabsComponent->getTabIndexByName("ComponentDebugger") );
	componentVisualiserComponent = nullptr;
}

void ValueTreeEditor::resized()
{
	DBGM("In ValueTreeEditor::resized() ");
	/ *Component* compDebugger = tabsComponent->getTabContentComponent(3);
	auto test = tabsComponent->getContentComponentByName("ComponentDebugger");
	if (compDebugger  != nullptr)
	{
		compDebugger->setSize(getWidth(), getHeight());
	}
	bufferVisualiserComponent->setSize(getWidth(), getHeight());* /
	
//	tabsComponent->setSize(getWidth(), getHeight());
	for (auto currComponent : tabsComponent->getTabContentComponentArray())
	{
		currComponent->setSize(getWidth(), getHeight());
	}

}*/

ValueTreeEditor::Editor::Editor(const String& editorName) :
	treeView(editorName),
	layoutResizer(&layout, 1, false)
{
	this->setName(editorName);
	layout.setItemLayout(0, -0.1, -0.9, -0.4);
	layout.setItemLayout(1, 5, 5, 5);
	layout.setItemLayout(2, -0.1, -0.9, -0.4);
	layout.setItemLayout(3, -0.1, -0.9, -0.2);

	addTraceLabel("Debug Label", "Debug");
	setSize(this->getParentWidth(), this->getParentHeight());
	treeView.setDefaultOpenness(true);
	addAndMakeVisible(treeView);
	addAndMakeVisible(propertyEditor);
	addAndMakeVisible(layoutResizer);
	addAndMakeVisible(labelsComponent);
	startTimer(100);
}

ValueTreeEditor::Editor::~Editor()
{
	treeView.deleteRootItem();
	treeView.setRootItem(nullptr);
	if (rootItem != nullptr)
		rootItem->setOwnerViewPublic(nullptr);
	rootItem = nullptr;
	theLabels.clear();

}

void ValueTreeEditor::Editor::resized()
{
	Component* comps[] = {&treeView, &layoutResizer, &propertyEditor, &labelsComponent};
	layout.layOutComponents(comps, 4, 0, 0, getWidth(), getHeight(), true, true);
}

void ValueTreeEditor::Editor::addTraceLabel(const String& inName, Value& theValue)
{
	addTraceLabel(inName, theValue.toString());
}

void ValueTreeEditor::Editor::addTraceLabel(const String& inName, const String& inText)
{
	Label* theLabel = new Label(inName, formatLabelText(inName, inText));

	theLabels.add(theLabel);
	theLabel->setFont(Font(12.00f, Font::bold));
	theLabel->setJustificationType(Justification::centredLeft);
	theLabel->setEditable(false, false, false);
	theLabel->setColour(Label::textColourId, Colours::black);
	theLabel->setColour(TextEditor::textColourId, Colours::black);
	theLabel->setColour(TextEditor::backgroundColourId, Colour(0x00FFFF00));
	theLabel->setTopLeftPosition(((labelsComponent.getNumChildComponents() - 1) / 4) * 150, ((labelsComponent.getNumChildComponents() - 1) % 4) * 15);
	theLabel->setSize(150, 15);

	labelsComponent.addAndMakeVisible(theLabel);
	resized();
}

void ValueTreeEditor::Editor::addOrSetTraceLabel(const String& inName, const String& inText)
{
	labelsAddSetBufferMap.insert_or_assign(inName, inText);
}

void ValueTreeEditor::Editor::removeTraceLabel(const String& inName)
{	
	labelRemoveBuffer.push_back(inName);
}

void ValueTreeEditor::Editor::setLabelText(const String& labelName, const float inText)
{
	setLabelText(labelName, String(inText));
}

bool ValueTreeEditor::Editor::setLabelText(const String& labelName, const String& inText)
{
	bool labelFoundAndSet = false;
	for (int labelPointer = 0; labelPointer < theLabels.size(); labelPointer++)
	{
		if (theLabels[labelPointer]->getName() == labelName)
		{
			String tempStr(formatLabelText(labelName, inText));
			theLabels[labelPointer]->setText(tempStr, dontSendNotification);
			labelFoundAndSet = true;
		}
	}
	return labelFoundAndSet;
}

String ValueTreeEditor::Editor::formatLabelText(const String& labelName, const String& inText)
{
	return labelName + ":  " + inText;
}

void ValueTreeEditor::Editor::setTree(ValueTree newTree)
{
	if (newTree == ValueTree::invalid)
	{
		treeView.setRootItem(nullptr);
	} else if (tree != newTree)
	{
		tree = newTree;
		rootItem = new Item(&propertyEditor, tree);
		treeView.setRootItem(rootItem);
	}
}

void ValueTreeEditor::Editor::timerCallback()
{
	//DBGM("In timer callback");

	//Skip deleting events if nothing needs to be removed
	if (!labelRemoveBuffer.empty())
	{		
		while(! labelRemoveBuffer.empty())
		{
			labelsAddSetBufferMap.erase(labelRemoveBuffer.back());
			labelRemoveBuffer.pop_back();
		}
	}
	theLabels.clearQuick(true);

	//build label array from map
	for (auto& labelMapElement : labelsAddSetBufferMap)
	{
		bool labelExistsAndWasChanged = setLabelText(labelMapElement.first, labelMapElement.second);
		if (!labelExistsAndWasChanged)
			addTraceLabel(labelMapElement.first, labelMapElement.second);
	}

}



String ValueTreeEditor::Item::getUniqueName() const
{
	if (t.getParent() == ValueTree::invalid) return "1";

	return String(t.getParent().indexOf(t));
}

void ValueTreeEditor::Item::valueTreeRedirected(ValueTree& treeWhichHasBeenChanged)
{
	if (treeWhichHasBeenChanged == t)
		updateSubItems();

	treeHasChanged();
}

void ValueTreeEditor::Item::valueTreeParentChanged(ValueTree& treeWhoseParentHasChanged)
{
	treeHasChanged();
}

void ValueTreeEditor::Item::valueTreeChildOrderChanged(ValueTree& parentTreeWhoseChildrenHaveMoved, int, int)
{
	if (parentTreeWhoseChildrenHaveMoved == t)
		updateSubItems();

	treeHasChanged();
}

void ValueTreeEditor::Item::valueTreeChildRemoved(ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int)
{
	if (parentTree == t)
		updateSubItems();

	treeHasChanged();
}

void ValueTreeEditor::Item::valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded)
{
	if (parentTree == t)
		updateSubItems();

	treeHasChanged();
}

void ValueTreeEditor::Item::valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property)
{
	if (t != treeWhosePropertyHasChanged) return;

	t.removeListener(this);
//            if (isSelected())
//                propertiesEditor->setSource(t);
	repaintItem();
	t.addListener(this);
}

void ValueTreeEditor::Item::itemSelectionChanged(bool isNowSelected)
{
	if (isNowSelected)
	{
		t.removeListener(this);
		propertiesEditor->setSource(t);
		t.addListener(this);
	}
}

void ValueTreeEditor::Item::paintItem(Graphics& g, int w, int h)
{
	Font font;
	Font smallFont(11.0);


	if (isSelected())
		g.fillAll(Colours::white);


	const float padding = 20.0f;

	String typeName = t.getType().toString();

	const float nameWidth = font.getStringWidthFloat(typeName);
	const float propertyX = padding + nameWidth;

	g.setColour(Colours::black);

	g.setFont(font);

	g.drawText(t.getType().toString(), 0, 0, w, h, Justification::left, false);
	g.setColour(Colours::blue);

	String propertySummary;

	for (int i = 0; i < t.getNumProperties(); ++i)
	{
		const Identifier name = t.getPropertyName(i).toString();
		String propertyValue = t.getProperty(name).toString();;
#ifdef JCF_SERIALIZER

		if (t[name].isObject())
		{
			ReferenceCountedObject* p = t[name].getObject();

			if (Serializable* s = dynamic_cast<Serializable*> (p))
				propertyValue = "[[" + s->getDebugInformation() + "]]";
		}

#endif
		if (name.toString().equalsIgnoreCase("name"))
		{
			//propertySummary += " " + name.toString() + "=\"" + propertyValue + "\"";
		} else
		{
			propertySummary += " " + name.toString() + "=" + propertyValue.toStdString().substr(0, 8);
		}
	}

	g.drawText(propertySummary, propertyX, 0, w - propertyX, h, Justification::left, true);
}

void ValueTreeEditor::Item::updateSubItems()
{
	ScopedPointer<XmlElement> openness = getOpennessState();
	clearSubItems();
	int children = t.getNumChildren();

	for (int i = 0; i < children; ++i)
		addSubItem(new Item(propertiesEditor, t.getChild(i)));

	if (openness)
		restoreOpennessState(*openness);
}

void ValueTreeEditor::Item::itemOpennessChanged(bool isNowOpen)
{
	if (isNowOpen) updateSubItems();
}

bool ValueTreeEditor::Item::mightContainSubItems()
{
	return t.getNumChildren() > 0;
}

ValueTreeEditor::Item::~Item()
{
	clearSubItems();
}

ValueTreeEditor::Item::Item(PropertyEditor* inPropertiesEditor, ValueTree tree) :
	t(tree)
{
	t.addListener(this);
	propertiesEditor = inPropertiesEditor;
}

ValueTreeEditor::PropertyEditor::PropertyEditor()
{
	noEditValue = "not editable";
}

void ValueTreeEditor::PropertyEditor::setSource(ValueTree& tree)
{
	clear();

	t = tree;

	const int maxChars = 200;

	Array<PropertyComponent*> pc;

	for (int i = 0; i < t.getNumProperties(); ++i)
	{
		const Identifier name = t.getPropertyName(i).toString();
		Value v = t.getPropertyAsValue(name, nullptr);
		TextPropertyComponent* tpc;

		if (v.getValue().isObject())
		{
			tpc = new TextPropertyComponent(noEditValue, name.toString(), maxChars, false);
			tpc->setEnabled(false);
		} else
		{
			tpc = new TextPropertyComponent(v, name.toString(), maxChars, false);
		}

		pc.add(tpc);
	}

	addProperties(pc);
}
} //Namespace Zen