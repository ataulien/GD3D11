#include "pch.h"
#include "D2DMessageBox.h"
#include "SV_Label.h"
#include "SV_Panel.h"
#include "D2DView.h"

D2DMessageBox::D2DMessageBox(D2DView* view, D2DSubView* parent, ED2D_MB_TYPE type) : D2DDialog(view, parent)
{
	Callback = NULL;
	CallbackUserData = NULL;

	OkButton = NULL;
	YesButton = NULL;
	NoButton = NULL;

	InitControls(type);
}


D2DMessageBox::~D2DMessageBox(void)
{
}

/** Initializes the controls of this view */
XRESULT D2DMessageBox::InitControls(ED2D_MB_TYPE type)
{
	D2DSubView::InitControls();



	// Create message label
	Message = new SV_Label(MainView, MainPanel);
	Message->SetSize(GetSize());
	Message->AlignUnder(Header, 5.0f);
	Message->SetRect(D2D1::RectF(5.0f, Message->GetRect().top, ViewRect.right - 5.0f, ViewRect.bottom - 5.0f));
	Message->SetCaption("Text");

	SetPositionCentered(D2D1::Point2F(MainView->GetRenderTarget()->GetSize().width / 2, MainView->GetRenderTarget()->GetSize().height / 2), D2D1::SizeF(400, 150));

	switch(type)
	{
	case D2D_MBT_YES_NO:
		YesButton = AddButton("Yes", ButtonPressed, this);
		NoButton = AddButton("No", ButtonPressed, this);
		break;

	case D2D_MBT_OK:
	default:
		OkButton = AddButton("Ok", ButtonPressed, this);
		break;
	}

	return XR_SUCCESS;
}

void D2DMessageBox::ButtonPressed(SV_Button* sender, void* userdata)
{
	D2DMessageBox* b = (D2DMessageBox *)userdata;

	if(sender == b->OkButton)
	{
		b->SetHidden(true);

		if(b->Callback)
			b->Callback(D2D_MB_OK, b->CallbackUserData);
	}else if(sender == b->YesButton)
	{
		b->SetHidden(true);

		if(b->Callback)
			b->Callback(D2D_MB_YES, b->CallbackUserData);
	}else if(sender == b->NoButton)
	{
		b->SetHidden(true);

		if(b->Callback)
			b->Callback(D2D_MB_NO, b->CallbackUserData);
	}

	// The view checks the messagebox every frame for visibility
	// If one is hidden it will get removed
}

/** Sets the callback for this box */
void D2DMessageBox::SetCallback(D2DMessageBoxCallback callback, void* userdata)
{
	Callback = callback;
	CallbackUserData = userdata;
}

/** Sets the text displayed in the messagebox */
void D2DMessageBox::SetMessage(const std::string& text)
{
	Message->SetCaption(text);
}

/** Sets the position and size of this sub-view */
void D2DMessageBox::SetRect(const D2D1_RECT_F& rect)
{
	D2DDialog::SetRect(rect);

	Message->SetSize(GetSize());
}