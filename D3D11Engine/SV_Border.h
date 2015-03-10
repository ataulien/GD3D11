#pragma once
#include "d2dsubview.h"
class SV_Border :
	public D2DSubView
{
public:
	SV_Border(D2DView* view, D2DSubView* parent);
	~SV_Border(void);

	/** Sets the border color */
	void SetBorderColor(const D2D1_COLOR_F& color);

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAb, float deltaTimes);

protected:
	/** Border color */
	D2D1_COLOR_F BorderColor;
};

