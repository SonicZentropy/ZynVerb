
#ifndef Zen_component_debugger_h
#define Zen_component_debugger_h

#include "JuceHeader.h"

namespace Zen
{

/**
 Include one of these and attach it to a component you want to use
 as the root (usually your main window) and a second window will
 appear showing a tree view of all your components.

 e.g.

 ScopedPointer<jcf::ComponentDebugger> debugger;

 Then something like:

 MainComponent::MainComponent()
 {
	debugger = new jcf::ComponentDebugger(this);
 }

 When you click on a component in the debugger's list a box will
 be drawn on your software showing the bounds of the component.

 Components marked as not-visible will be shown in grey.

 Components with zero sizes will be highlighted in red.

 Components partially or completely outside their parents bounds
 will be marked in yellow.

 @note
 This code isn't pretty, and the UI isn't pretty - it's for debugging
 and definitely not production use!
 */

class ComponentBoundsEditor;

class ComponentDebugger
	: public Component
{
public:
	class Debugger;

	class ComponentTreeViewItem
		:
		public TreeViewItem
	{
	public:
		ComponentTreeViewItem(Debugger * owner, Component * c);
		~ComponentTreeViewItem();

		bool mightContainSubItems() override;
		void paintItem(Graphics & g, int w, int h) override;
		void itemSelectionChanged(bool nowSelected) override;
	private:
		String name;
		String type;
		bool isVisible;

		int numChildren;

		Debugger* owner;

		bool zeroSizeFlag;
		bool outsideBoundsFlag;

		WeakReference<Component> component;
	};

	/**
	 The main debugger component.  Attached to the ComponentDebugger document window.
	 */
	class Debugger
		:public Component,
		public Button::Listener
	{
	public:
		explicit Debugger(Component * rootComponent, int componentWidth = -54321, int componentHeight = -54321);

		~Debugger();

		void buttonClicked(Button *) override;
		void refresh();

		/**
		 Returns the location of the component in the root component's
		 coordinate space.
		 */
		Rectangle<int> getLocationOf(Component * c);

		/**
		 A component which draws over your existing application to show where
		 a selected component is positioned.

		 @todo - allow the component to be dragged using this.
		 */
		class Highlighter
			:
			public Component
		{
		public:
			Highlighter()
			{
				setInterceptsMouseClicks(false, false);
			}
			void paint(Graphics & g) override
			{
				g.setColour(Colours::red);
				g.drawRect(getLocalBounds());
			}
		};

		void setComponentToEdit(Component *);

		void setHighlight(Component * component)
		{
			highlight = new Highlighter();
			highlight->setBounds(getLocationOf(component));
			root->addAndMakeVisible(highlight);
		}

	private:
		void resized() override;
		void paint(Graphics & g) override { g.fillAll(Colours::lightgrey); }
		void copyBoundsToClipboard();

		ScopedPointer<Highlighter> highlight;
		ScopedPointer<ComponentBoundsEditor> boundsEditor;

		/* We use the default look and feel just in case your application has some
		 wacky and unreadable alternative in use. */
		LookAndFeel_V3 lookAndFeel;

		Component * root;

		TextButton refreshButton;
		TextButton copyBoundsButton;
		TextButton hideHighlightButton;
		TreeView tree;

		JUCE_DECLARE_NON_COPYABLE(Debugger)
	};

	/**
	 Show a component debugger, using root as the starting point.
	 */
	explicit ComponentDebugger(Component * root)
		: Component("ComponentDebugger"),
		debugger(root)
	{		
		debugger.setSize(this->getParentWidth(), this->getParentHeight());
		addAndMakeVisible(debugger);
		debugger.refresh();
	}

	ComponentDebugger(Component* root, int componentWidth, int componentHeight, const String& componentName = "ComponentDebugger")
		:Component(componentName),
		debugger(root, componentWidth, componentHeight)
	{
		debugger.setSize(componentWidth, componentHeight);
		addAndMakeVisible(debugger);
		debugger.refresh();
	}

	void refresh()
	{
		debugger.refresh();
	}

	virtual void resized() override;


private:
	Debugger debugger;
};


} // Namespace Zen

#endif //Zen_Component_Debugger
