#pragma once
#include "d2dsubview.h"
#include "SV_Button.h"

struct DLG_ButtonInfo
{
	SV_Button* Button;
	SV_ButtonPressedCallback* Callback;
	void* Userdata;
};

class SV_Panel;
class SV_Button;
class SV_Label;
class D2DDialog :
	public D2DSubView
{
public:
	D2DDialog(D2DView* view, D2DSubView* parent);
	~D2DDialog(void);

	/** Initializes the controls of this view */
	virtual XRESULT InitControls();

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Updates the subview */
	void Update(float deltaTime);

	/** Processes a window-message. Return false to stop the message from going to children */
	virtual bool OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs);

	/** Adds a button to the right side of the dialog */
	SV_Button* AddButton(const std::string& caption, SV_ButtonPressedCallback callback, void* userdata, float width = 85);

	/** Sets the position and size of this sub-view */
	virtual void SetRect(const D2D1_RECT_F& rect);

	/** Sets the header-text */
	void SetHeaderText(const std::string& text);

	/** Sets this dialog into the center of the screen, with the given size */
	void SetPositionCentered(const D2D1_POINT_2F& position, const D2D1_SIZE_F& size);
protected:
	std::vector<SV_Button*> Buttons;

	SV_Panel* MainPanel;
	SV_Label* Header;

	bool DraggingWindow;
	D2D1_POINT_2F WindowDragOffset;
};

