#include "pch.h"
#include "WidgetContainer.h"
#include "BaseWidget.h"
#include "Widget_TransRot.h"
#include "WorldConverter.h"

WidgetContainer::WidgetContainer(void)
{
	Widgets.push_back(new Widget_TransRot(this));
}


WidgetContainer::~WidgetContainer(void)
{
	Toolbox::DeleteElements(Widgets);
}

/** Renders the active widget */
void WidgetContainer::Render()
{
	for(auto it = Widgets.begin(); it != Widgets.end(); it++)
	{
		(*it)->RenderWidget();
	}
}

/** Adds a selectiontarget */
void WidgetContainer::AddSelection(BaseVobInfo* vob)
{
	Selection.insert(vob);
	
	for(auto it = Widgets.begin(); it != Widgets.end(); it++)
	{
		(*it)->OnSelectionAdded(vob->Vob);
	}
}

/** Clears the selection */
void WidgetContainer::ClearSelection()
{
	Selection.clear();
}

/** Removes a single object from the selection */
void WidgetContainer::RemoveSelection(BaseVobInfo* vob)
{
	Selection.erase(vob);
}

/** Returns the current selection-set */
std::set<BaseVobInfo*>& WidgetContainer::GetSelection()
{
	return Selection;
}

/** Returns if one of the widgets is currently clicked */
bool WidgetContainer::IsWidgetClicked()
{
	for(auto it = Widgets.begin(); it != Widgets.end(); it++)
	{
		if((*it)->IsActive())
			return true;
	}

	return false;
}

/** Called when the owning window got a message */
void WidgetContainer::OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for(auto it = Widgets.begin(); it != Widgets.end(); it++)
	{
		(*it)->OnWindowMessage(hWnd, msg, wParam, lParam);
	}
}