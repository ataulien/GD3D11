#pragma once
#include "d2ddialog.h"

enum ED2D_MB_ACTION
{
	D2D_MB_OK = 0,
	D2D_MB_YES = 1,
	D2D_MB_NO = 2
};

enum ED2D_MB_TYPE
{
	D2D_MBT_OK = 0,
	D2D_MBT_YES_NO = 1
};

typedef void (__cdecl* D2DMessageBoxCallback)(ED2D_MB_ACTION, void*);


class D2DMessageBox :
	public D2DDialog
{
public:
	D2DMessageBox(D2DView* view, D2DSubView* parent, ED2D_MB_TYPE type);
	~D2DMessageBox(void);

	/** Sets the text displayed in the messagebox */
	void SetMessage(const std::string& text);

	/** Sets the callback for this box */
	void SetCallback(D2DMessageBoxCallback callback, void* callbackUserData);

	/** Initializes the controls of this view */
	virtual XRESULT InitControls(ED2D_MB_TYPE type);

	/** Sets the position and size of this sub-view */
	void SetRect(const D2D1_RECT_F& rect);

protected:
	static void ButtonPressed(SV_Button* sender, void* userdata);
	
	/** Text-Label */
	SV_Label* Message;

	/** Ok */
	SV_Button* OkButton;

	/** Yes */
	SV_Button* YesButton;

	/** No */
	SV_Button* NoButton;

	/** Callback */
	D2DMessageBoxCallback Callback;
	void* CallbackUserData;
};

