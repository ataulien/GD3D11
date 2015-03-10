#include "pch.h"
#include "D2DDialog.h"
#include "SV_Button.h"
#include "SV_Panel.h"
#include "SV_Label.h"
#include "D2DView.h"

const float DIALOG_HEADER_SIZE = 25.0f;

D2DDialog::D2DDialog(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	DraggingWindow = false;

	MainPanel = new SV_Panel(view, this);
	MainPanel->SetRect(ViewRect);
	MainPanel->SetGlossyOutline(true);
	MainPanel->SetPanelShadow(true, 40.0f);

	Header = new SV_Label(view, MainPanel);
	Header->SetPositionAndSize(D2D1::Point2F(0,0), D2D1::SizeF(GetSize().width, DIALOG_HEADER_SIZE));
	Header->SetCaption("Dialog");
	Header->SetDrawBackground(true);
	Header->SetHorizAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	Header->SetVertAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	InitControls();
}


D2DDialog::~D2DDialog(void)
{
}


/** Initializes the controls of this view */
XRESULT D2DDialog::InitControls()
{
	D2DSubView::InitControls();

	return XR_SUCCESS;
}

/** Draws this sub-view */
void D2DDialog::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	D2DSubView::Draw(clientRectAbs, deltaTime);
}

/** Updates the subview */
void D2DDialog::Update(float deltaTime)
{
	D2DSubView::Update(deltaTime);
}

/** Processes a window-message. Return false to stop the message from going to children */
bool D2DDialog::OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs)
{
	switch(msg)
	{
	case WM_LBUTTONDOWN:
		{
			POINT p = D2DView::GetCursorPosition();
			if(PointInsideRect(p, D2D1::RectF(clientRectAbs.left, clientRectAbs.top, clientRectAbs.right, clientRectAbs.top + DIALOG_HEADER_SIZE)))
			{
				DraggingWindow = true;
				WindowDragOffset = D2D1::Point2F(p.x - clientRectAbs.left, p.y - clientRectAbs.top);
				return false;
			}
		}
		break;

	case WM_MOUSEMOVE:
		if(DraggingWindow)
		{
			POINT p = D2DView::GetCursorPosition();

			SetPosition(D2D1::Point2F(p.x - WindowDragOffset.x, p.y - WindowDragOffset.y));
			return false;
		}
		break;

	case WM_LBUTTONUP:
		if(DraggingWindow)
		{
			DraggingWindow = false;
			return false;
		}
		break;
	}

	return D2DSubView::OnWindowMessage(hWnd, msg, wParam, lParam, clientRectAbs);
}

/** Adds a button to the right side of the dialog */
void D2DDialog::AddButton(const std::string& caption, SV_ButtonPressedCallback callback, void* userdata, float width)
{
	SV_Button* button = new SV_Button(MainView, MainPanel);

	button->SetPositionAndSize(D2D1::Point2F(MainPanel->GetSize().width - (width + 5), MainPanel->GetSize().height - 25), D2D1::SizeF(width, 20));
	button->SetPressedCallback(callback, userdata);
	button->SetCaption(caption);

	if(!Buttons.empty())
		button->AlignLeftTo(Buttons.back(), 5);

	Buttons.push_back(button);
}


/** Sets the position and size of this sub-view */
void D2DDialog::SetRect(const D2D1_RECT_F& rect)
{
	D2DSubView::SetRect(rect);

	// Update panel size
	MainPanel->SetSize(GetSize());
	Header->SetPositionAndSize(D2D1::Point2F(1,1), D2D1::SizeF(GetSize().width-2, 20-2));
}

/** Sets the header-text */
void D2DDialog::SetHeaderText(const std::string& text)
{
	Header->SetCaption(text);
}

/** Sets this dialog into the center of the screen, with the given size */
void D2DDialog::SetPositionCentered(const D2D1_POINT_2F& position, const D2D1_SIZE_F& size)
{
	D2D1_POINT_2F p = D2D1::Point2F(position.x - size.width / 2, position.y - size.height / 2);

	SetPositionAndSize(p, size);
}