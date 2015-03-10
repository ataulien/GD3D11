#pragma once
#include "d2dsubview.h"

class SV_TabControl;
typedef void (__cdecl* SV_TabSwitchedCallback)(SV_TabControl*, void*);

struct SV_TabControl_Tab
{
	SV_TabControl_Tab();
	~SV_TabControl_Tab();

	IDWriteTextLayout* CaptionLayout;
	std::string Caption;
	std::list<D2DSubView*> Controls;
};

class SV_Panel;
class SV_TabControl : 
	public D2DSubView
{
public:
	SV_TabControl(D2DView* view, D2DSubView* parent);
	~SV_TabControl(void);

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Processes a window-message. Return false to stop the message from going to children */
	virtual bool OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs);

	/** Adds a control to a tab */
	void AddControlToTab(D2DSubView* control, const std::string& tab);

	/** Returns the tab-panel of this TabControl. All subviews must be added to this panel. */
	SV_Panel* GetTabPanel();

	/** Sets the position and size of this sub-view */
	virtual void SetRect(const D2D1_RECT_F& rect);

	/** Sets active tab */
	void SetActiveTab(const std::string& tab);

	/** Sets the callback */
	void SetTabSwitchedCallback(SV_TabSwitchedCallback fn, void* userdata);

	/** Returns the name of the active tab */
	std::string GetActiveTab();

	/** If true, this will only show the currently active tab */
	void SetOnlyShowActiveTab(bool value);
protected:
	/** Sets the caption of a tab */
	void SetTabCaption(const std::string& caption, SV_TabControl_Tab* tab);

	/** Shows/Hides all controls in a SV_TabControl_Tab */
	void SetTabVisibility(SV_TabControl_Tab* tab, bool hide);

	/** Map of tabs */
	std::map<std::string, SV_TabControl_Tab> Tabs;

	/** Main panel */
	SV_Panel* TabPanel;

	/** Active tab */
	SV_TabControl_Tab* ActiveTab;
	bool OnlyShowActiveTab;

	/** Callback */
	SV_TabSwitchedCallback TabSwitchedCallback;
	void* TabSwitchedUserdata;
};

