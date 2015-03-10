#pragma once
#include "d2dsubview.h"
#include <dwrite_1.h>

class SV_Label :
	public D2DSubView
{
public:
	SV_Label(D2DView* view, D2DSubView* parent);
	~SV_Label(void);

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Sets the position and size of this sub-view */
	virtual void SetRect(const D2D1_RECT_F& rect);

	/** Sets this labels caption */
	void SetCaption(const std::string& caption);

	/** Sets the text size */
	void SetTextSize(float size);

	/** Sets the text color */
	void SetTextColor(const D2D1_COLOR_F& color);

	/** Sets the horizontal alignment of this text */
	void SetHorizAlignment(DWRITE_TEXT_ALIGNMENT alignment);

	/** Sets the horizontal alignment of this text */
	void SetVertAlignment(DWRITE_PARAGRAPH_ALIGNMENT alignment);

	/** Sets if this text should have a background */
	void SetDrawBackground(bool bgr);
protected:
	/** Current Caption */
	std::string Caption;

	/** Draw black background? */
	bool DrawBackground;

	/** Text layout */
	IDWriteTextLayout* CaptionLayout;

	/** Alignment */
	DWRITE_TEXT_ALIGNMENT HorizAlignment;
	DWRITE_PARAGRAPH_ALIGNMENT VertAlignment;
	float TextSize;
	D2D1_COLOR_F TextColor;
};

