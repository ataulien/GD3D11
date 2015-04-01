#include "pch.h"
#include "SV_Label.h"
#include "D2DView.h"

SV_Label::SV_Label(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	CaptionLayout = NULL;
	VertAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
	HorizAlignment = DWRITE_TEXT_ALIGNMENT_LEADING;
	DrawBackground = false;

	TextSize = 12;
	TextColor = D2D1::ColorF(1,1,1,1);
}


SV_Label::~SV_Label(void)
{
	if(CaptionLayout)CaptionLayout->Release();
}

/** Draws this sub-view */
void SV_Label::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

	//Draw text
	if(CaptionLayout)
	{
		if(DrawBackground)
		{
			MainView->GetBrush()->SetColor(D2D1::ColorF(0,0,0,0.7f));

			if(HorizAlignment == DWRITE_TEXT_ALIGNMENT_LEADING)
			{
				MainView->GetRenderTarget()->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(ViewRect.left - 1, 
					ViewRect.top, 
					ViewRect.left + D2DView::GetLabelTextWidth(CaptionLayout, Caption) + 1, 
					ViewRect.top + D2DView::GetTextHeight(CaptionLayout, Caption) + 3), 2, 2), MainView->GetBrush());
			}else
			{
				MainView->GetRenderTarget()->FillRoundedRectangle(D2D1::RoundedRect(ViewRect, 2, 2), MainView->GetBrush());
			}

		}

		MainView->GetBrush()->SetColor(TextColor);
		MainView->GetRenderTarget()->DrawTextLayout(D2D1::Point2F(ViewRect.left, ViewRect.top),CaptionLayout, MainView->GetBrush());
	}

	D2DSubView::Draw(clientRectAbs, deltaTime);
}

/** Sets this buttons caption */
void SV_Label::SetCaption(const std::string& caption)
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

	if(!CaptionLayout)
		return;

	DWRITE_TEXT_RANGE range;
	range.startPosition = 0;
	range.length = caption.size();
	CaptionLayout->SetFontSize(TextSize, range);

	if(CaptionLayout)
	{
		SetVertAlignment(VertAlignment);
		SetHorizAlignment(HorizAlignment);
	}
	else
	{
		LogWarn() << "Failed to create TextLayout for caption '" << caption << "'";
	}
}

/** Sets the position and size of this sub-view */
void SV_Label::SetRect(const D2D1_RECT_F& rect)
{
	D2D1_RECT_F r = ViewRect;

	D2DSubView::SetRect(rect);

	// check for actual change
	if(memcmp(&r, &rect, sizeof(rect)) != 0)
	{
		// Need to update the layout
		SetCaption(Caption);
	}
}

/** Sets the horizontal alignment of this text */
void SV_Label::SetHorizAlignment(DWRITE_TEXT_ALIGNMENT alignment)
{
	HorizAlignment = alignment;

	if(CaptionLayout)
		CaptionLayout->SetTextAlignment(alignment);
		
}

/** Sets the horizontal alignment of this text */
void SV_Label::SetVertAlignment(DWRITE_PARAGRAPH_ALIGNMENT alignment)
{
	VertAlignment = alignment;

	if(CaptionLayout)
		CaptionLayout->SetParagraphAlignment(alignment);
}

/** Sets if this text should have a background */
void SV_Label::SetDrawBackground(bool bgr)
{
	DrawBackground = bgr;
}

/** Sets the text size */
void SV_Label::SetTextSize(float size)
{
	if(TextSize != size)
	{
		TextSize = size;

		DWRITE_TEXT_RANGE range;
		range.startPosition = 0;
		range.length = Caption.size();

		if(CaptionLayout)
			CaptionLayout->SetFontSize(TextSize, range);
	}
}

/** Sets the text color */
void SV_Label::SetTextColor(const D2D1_COLOR_F& color)
{
	TextColor = color;
}