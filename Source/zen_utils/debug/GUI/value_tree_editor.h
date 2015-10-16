#ifndef VALUE_TREE_EDITOR_H_INCLUDED
#define VALUE_TREE_EDITOR_H_INCLUDED

#include "JuceHeader.h"
#include <map>
#include "buffer_visualiser.h"
#include "component_debugger.h"
#include "ZenMidiVisualiserComponent.h"
#include "../../components/NotepadComponent/NotepadComponent.h"

namespace Zen
{
/**
Display a separate desktop window for viewed and editing a value
tree's property fields.

Instantate a ValueTreeEditor instance, then call
ValueTreeEditor::setSource(ValueTree &) and it'll display your
tree.

For example:

@code valueTreeEditor = new ValueTreeEditor();
valueTreeEditor->setSource(myTree);

@note This code isn't pretty, and the UI isn't pretty - it's for
debugging and definitely not production use!
*/

//class ValueTreeEditor;
class ValueTreeEditor :
	public DocumentWindow
{


public:
	class Editor;
	explicit ValueTreeEditor(int componentWidth = 500, int componentHeight = 500);

	~ValueTreeEditor();

	
private:
	ScopedPointer<TabbedComponent> tabsComponent;
	ScopedPointer<Editor> valueTreeEditorComponent;
	ScopedPointer<ZenMidiVisualiserComponent> midiVisualiserComponent;
	ScopedPointer<BufferVisualiser> bufferVisualiserComponent;
	ScopedPointer<ComponentDebugger> componentVisualiserComponent;
	ScopedPointer<NotepadComponent> notepadComponent;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueTreeEditor);

public:
/** CLASS--PROPERTYEDITOR===================================================================================== */
	class PropertyEditor :
		public PropertyPanel
	{
	public:
		PropertyEditor();
		void setSource(ValueTree& tree);

	private:
		Value noEditValue;
		ValueTree t;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PropertyEditor);
	};

/** CLASS--ITEM===================================================================================== */
	class Item :
		public TreeViewItem,
		public ValueTree::Listener
	{
	public:
		Item(PropertyEditor* inPropertiesEditor, ValueTree tree);

		~Item();

		bool mightContainSubItems() override;

		void itemOpennessChanged(bool isNowOpen) override;

		void updateSubItems();

		void paintItem(Graphics& g, int w, int h) override;

		void itemSelectionChanged(bool isNowSelected) override;


		/* Enormous list of ValueTree::Listener options... */
		void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;
		void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded) override;


		void valueTreeChildRemoved(ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int) override;
		void valueTreeChildOrderChanged(ValueTree& parentTreeWhoseChildrenHaveMoved, int, int) override;
		void valueTreeParentChanged(ValueTree& treeWhoseParentHasChanged) override;
		void valueTreeRedirected(ValueTree& treeWhichHasBeenChanged) override;

		/* Works only if the ValueTree isn't updated between calls to getUniqueName. */
		String getUniqueName() const override;

	private:	
		PropertyEditor* propertiesEditor;
		ValueTree t;
		Array<Identifier> currentProperties;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Item);
	};
public:

/** CLASS--EDITOR===================================================================================== */
	class Editor :
		public Component, public Timer
	{
	public:

		explicit Editor(const String& editorName = "ValueTreeEditor_Editor");
		~Editor();

		void resized() override;

		void addTraceLabel(const String& inName, const String& inText);

		void addTraceLabel(const String& inName, Value& theValue);

		void addOrSetTraceLabel(const String& inName, const String& inText);

		void removeTraceLabel(const String& inName);
		bool setLabelText(const String& labelName, const String& inText);

		String formatLabelText(const String& labelName, const String& inText);
		void setLabelText(const String& labelName, const float inText);

		void setTree(ValueTree newTree);

	protected:
		virtual void timerCallback() override;

	public:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor);

		ScopedPointer<Item> rootItem;
		ValueTree tree;
		TreeView treeView;
		PropertyEditor propertyEditor;
		Component labelsComponent;
		OwnedArray<Label> theLabels;
		
		StretchableLayoutManager layout;
		StretchableLayoutResizerBar layoutResizer;

	protected:
		//Slower than unordered, but doesn't require a hash function
		//and since this is only used to Debug, it doesn't need any optimization
		std::map<String, String> labelsAddSetBufferMap;
		std::vector<String> labelRemoveBuffer;

	}; // CLASS Editor

}; // Class ValueTreeEditor
} // Namespace Zen
#endif  // VALUE_TREE_EDITOR_H_INCLUDED
