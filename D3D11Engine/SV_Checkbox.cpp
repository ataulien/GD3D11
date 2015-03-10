#include "pch.h"
#include "SV_Checkbox.h"
#include "D2DView.h"

SV_Checkbox::SV_Checkbox(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	CaptionLayout = NULL;
	DataToUpdate = NULL;
	IsChecked = false;
}


SV_Checkbox::~SV_Checkbox(void)
{
	if(CaptionLayout)CaptionLayout->Release();
}

/** Draws this sub-view */
void SV_Checkbox::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

	D2D1_RECT_F br = D2D1::RectF(ViewRect.left,ViewRect.top,ViewRect.left+SV_CHKBOX_BOX_SIZE_XY,ViewRect.top+SV_CHKBOX_BOX_SIZE_XY);

	//Outer shadow
	MainView->DrawSmoothShadow(&br, 12, 0.5, false, 4);

	//Render the inner white piece
	MainView->GetBrush()->SetColor(D2D1::ColorF(1,1,1,1));
	MainView->GetRenderTarget()->FillRectangle(br,MainView->GetBrush());

	//A black outline
	MainView->GetBrush()->SetColor(D2D1::ColorF(0,0,0,1));
	MainView->GetRenderTarget()->DrawRectangle(br,MainView->GetBrush());

	//Inner shadow
	MainView->DrawSmoothShadow(&br, -7, 0.25, true);

	

	//Draw text
	if(CaptionLayout)
	{
		MainView->GetBrush()->SetColor(D2D1::ColorF(1,1,1,1));
		MainView->GetRenderTarget()->DrawTextLayout(D2D1::Point2F(br.left+SV_CHKBOX_BOX_SIZE_XY*1.5f,br.top),CaptionLayout, MainView->GetBrush());
	}

	if(IsChecked)
	{
		//Draw a cross when checked
		D2DView::ShrinkRect(&br,3);
		DrawCross(br);
	}

	D2DSubView::Draw(clientRectAbs, deltaTime);
}

/** Draws the cross */
void SV_Checkbox::DrawCross(const D2D1_RECT_F& r)
{
	MainView->GetBrush()->SetColor(D2D1::ColorF(0,0,0,1));
	MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(r.left,r.top), D2D1::Point2F(r.right,r.bottom),MainView->GetBrush(), 2);
	MainView->GetRenderTarget()->DrawLine(D2D1::Point2F(r.right,r.top), D2D1::Point2F(r.left,r.bottom),MainView->GetBrush(), 2);
}

/** Sets the state */
void SV_Checkbox::SetChecked(bool active)
{
	IsChecked = active;
	if(DataToUpdate)
		*DataToUpdate = IsChecked;
}

/** Returns the state */
bool SV_Checkbox::GetChecked()
{
	return IsChecked;
}

/** Sets this buttons caption */
void SV_Checkbox::SetCaption(const std::string& caption)
{
	Caption = caption;

	if(CaptionLayout)CaptionLayout->Release();
	MainView->GetWriteFactory()->CreateTextLayout(
		Toolbox::ToWideChar(caption).c_str(),      // The string to be laid out and formatted.
		caption.length(),  // The length of the string.
		MainView->GetDefaultTextFormat(),  // The text format to apply to the string (contains font information, etc).
		ViewRect.right - ViewRect.left,         // The width of the layout box.
		ViewRect.bottom - ViewRect.top,         // The height of the layout box.
		&CaptionLayout  // The IDWriteTextLayout interface pointer.
		);

	if(CaptionLayout)
	{
		CaptionLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		CaptionLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	else
	{
		LogWarn() << "Failed to create TextLayout for caption '" << caption << "'";
	}
}

/** Sets the data location to update with this checkbox */
void SV_Checkbox::SetDataToUpdate(bool* data)
{
	DataToUpdate = data;
}

/** Sets the position and size of this sub-view */
void SV_Checkbox::SetRect(const D2D1_RECT_F& rect)
{
	D2D1_RECT_F r = rect;

	// Restrict the vertical size to the size of the checkbox
	r.bottom = rect.top + SV_CHKBOX_BOX_SIZE_XY;

	D2DSubView::SetRect(r);

	// Need to update the layout
	SetCaption(Caption);
}

/** Processes a window-message. Return false to stop the message from going to children */
bool SV_Checkbox::OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs)
{
	switch(msg)
	{
	case WM_LBUTTONDOWN:
		{
			POINT p = D2DView::GetCursorPosition();
			if(PointInsideRect(D2D1::Point2F((float)p.x, (float)p.y), clientRectAbs))
			{
				SetChecked(!GetChecked());
				return false;
			}
		}
		break;

	case WM_LBUTTONUP:
		break;
	}

	return D2DSubView::OnWindowMessage(hWnd, msg, wParam, lParam, clientRectAbs);
}