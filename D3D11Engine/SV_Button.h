#pragma once
#include "d2dsubview.h"

class SV_Button;
typedef void (__cdecl* SV_ButtonPressedCallback)(SV_Button*, void*);

class SV_Button :
	public D2DSubView
{
public:
	SV_Button(D2DView* view, D2DSubView* parent);
	~SV_Button(void);

	/** Sets this buttons caption */
	void SetCaption(const std::string& caption);

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Sets the position and size of this sub-view */
	virtual void SetRect(const D2D1_RECT_F& rect);

	/** Processes a window-message. Return false to stop the message from going to children */
	virtual bool OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs);

	/** Sets the button state */
	virtual void SetPressed(bool pressed);

	/** Sets the callback */
	void SetPressedCallback(SV_ButtonPressedCallback fn, void* userdata);

protected:
	D2D1_COLOR_F OutlineColor;
	D2D1_COLOR_F InnerlineColor;
	D2D1_COLOR_F BrightEmbossColor;
	D2D1_COLOR_F ButtonColor;

	/** Wether this button is currently pressed or not */
	bool IsPressed;

	/** Current Caption */
	std::string Caption;

	/** Text layout */
	IDWriteTextLayout* CaptionLayout;

	/** Callbacks */
	SV_ButtonPressedCallback PressedCallback;
	void* PressedUserdata;
};

