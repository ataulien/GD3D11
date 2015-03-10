#pragma once
#include "d2dsubview.h"

class SV_Label;
class SV_Slider;
class SV_NamedSlider :
	public D2DSubView
{
public:
	SV_NamedSlider(D2DView* view, D2DSubView* parent);
	~SV_NamedSlider(void);

	SV_Label* GetLabel(){return Label;}
	SV_Slider* GetSlider(){return Slider;}

	/** Updates the Position and Size of this control. Call after changing one of the children */
	void UpdateDimensions();

	/** Sets the position and size of this sub-view */
	virtual void SetRect(const D2D1_RECT_F& rect);

protected:
	SV_Label* Label;
	SV_Slider* Slider;
};

