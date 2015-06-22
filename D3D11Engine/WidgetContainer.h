#pragma once
#include "pch.h"

struct BaseVobInfo;
class BaseWidget;
class WidgetContainer
{
public:
	WidgetContainer(void);
	~WidgetContainer(void);

	/** Renders the active widget */
	void Render();

	/** Adds a selectiontarget */
	void AddSelection(BaseVobInfo* vob);

	/** Clears the selection */
	void ClearSelection();

	/** Removes a single object from the selection */
	void RemoveSelection(BaseVobInfo* vob);

	/** Returns if one of the widgets is currently clicked */
	bool IsWidgetClicked();

	/** Called when the owning window got a message */
	void OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/** Returns the current selection-set */
	std::set<BaseVobInfo*>& GetSelection();
protected:
	std::set<BaseVobInfo*> Selection;

	std::vector<BaseWidget*> Widgets;
};

