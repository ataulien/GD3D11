#include "pch.h"
#include "SV_Button.h"
#include "D2DView.h"

SV_Button::SV_Button(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	ButtonColor = D2D1::ColorF(0.4f,0.4f,0.4f,1.0f);
	BrightEmbossColor = D2D1::ColorF(0.8f,0.8f,0.8f,1.0f);
	OutlineColor = D2D1::ColorF(0.5f,0.6f,0.9f,1.0f);
	InnerlineColor = D2D1::ColorF(0.3f,0.3f,0.6f,1.0f);

	IsPressed = false;

	CaptionLayout = NULL;

	SetCaption("Button");

	PressedCallback = NULL;
	PressedUserdata = NULL;
}


SV_Button::~SV_Button(void)
{
	if(CaptionLayout)CaptionLayout->Release();
}

/** Draws this sub-view */
void SV_Button::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));


	MainView->DrawSmoothShadow(&ViewRect, 20.0f, 0.8f, false, 7);

	// Draw backpanel
	MainView->GetGUIStyleLinearBrush()->SetStartPoint(D2D1::Point2F(0, ViewRect.top));
	MainView->GetGUIStyleLinearBrush()->SetEndPoint(D2D1::Point2F(0, ViewRect.bottom));
	MainView->GetRenderTarget()->FillRectangle(ViewRect, MainView->GetGUIStyleLinearBrush());

	// Draw outer metallic deco-line
	MainView->GetLinearReflectBrush()->SetStartPoint(D2D1::Point2F(0, ViewRect.top));
	MainView->GetLinearReflectBrush()->SetEndPoint(D2D1::Point2F(0, ViewRect.bottom));
	MainView->GetRenderTarget()->DrawRectangle(ViewRect,MainView->GetLinearReflectBrush());

	MainView->GetBrush()->SetColor( SV_DEF_INNER_LINE_COLOR );

	// Draw inner deco-line
	D2D1_RECT_F r = ViewRect;
	D2DView::ShrinkRect(&r,1);
	MainView->GetRenderTarget()->DrawRectangle(r,MainView->GetBrush());

	// Draw caption
	MainView->GetBrush()->SetColor( D2D1::ColorF(1,1,1,1) );

	D2D1_POINT_2F captionPos = D2D1::Point2F(ViewRect.left,ViewRect.top);
	if(IsPressed)
	{	
		captionPos.x -= 2.0f;
		captionPos.y -= 2.0f;

		// Make pressed text darker
		MainView->GetBrush()->SetColor( D2D1::ColorF(1,1,1,0.7f) );
	}
	MainView->GetRenderTarget()->DrawTextLayout(captionPos, CaptionLayout, MainView->GetBrush());

	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));
	if(IsPressed)
	{
		r = ViewRect;
		D2DView::ShrinkRect(&r,2);
		r.left-=1;
		r.top-=1;
		r.right+=1;
		r.bottom+=1;

		MainView->DrawSmoothShadow(&r,-10.0f,0.7f/*,D2DObjects->BrushCollection.LinearGlowBrush,D2DObjects->BrushCollection.RadialGlowBrush*/);
		
	}

	D2DSubView::Draw(clientRectAbs, deltaTime);
}

/** Sets this buttons caption */
void SV_Button::SetCaption(const std::string& caption)
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
		CaptionLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		CaptionLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	else
	{
		LogWarn() << "Failed to create TextLayout for caption '" << caption << "'";
	}
}

/** Sets the position and size of this sub-view */
void SV_Button::SetRect(const D2D1_RECT_F& rect)
{
	D2DSubView::SetRect(rect);

	// Need to update the layout
	SetCaption(Caption);
}

/** Processes a window-message. Return false to stop the message from going to children */
bool SV_Button::OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs)
{
	switch(msg)
	{
	case WM_LBUTTONDOWN:
		{
			POINT p = D2DView::GetCursorPosition();
			if(PointInsideRect(D2D1::Point2F((float)p.x, (float)p.y), clientRectAbs))
			{
				SetPressed(true);
				return false;
			}
		}
		break;

	case WM_LBUTTONUP:
		if(IsPressed)
		{
			POINT p = D2DView::GetCursorPosition();
			if(PointInsideRect(D2D1::Point2F((float)p.x, (float)p.y), clientRectAbs))
			{
				// Button callback here
				if(PressedCallback)
					PressedCallback(this, PressedUserdata);

				SetPressed(false);
				return false;
			}else
			{
				SetPressed(false);
				return false;
			}
		}else
		{
			SetPressed(false);
		}
		break;
	}

	return D2DSubView::OnWindowMessage(hWnd, msg, wParam, lParam, clientRectAbs);
}

/** Sets the button state */
void SV_Button::SetPressed(bool pressed)
{
	IsPressed = pressed;
}

/** Sets the callback */
void SV_Button::SetPressedCallback(SV_ButtonPressedCallback fn, void* userdata)
{
	PressedCallback = fn;
	PressedUserdata = userdata;
}