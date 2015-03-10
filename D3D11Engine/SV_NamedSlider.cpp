#include "pch.h"
#include "SV_NamedSlider.h"
#include "SV_Label.h"
#include "SV_Slider.h"

SV_NamedSlider::SV_NamedSlider(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	Slider = new SV_Slider(view, this);
	Slider->SetRect(D2D1::RectF(0, 0, 200, 10));

	Label = new SV_Label(view, this);
	Label->SetRect(D2D1::RectF(0, 0, 200, 10));
	Label->SetCaption("Default");
}


SV_NamedSlider::~SV_NamedSlider(void)
{
}

/** Updates the Position and Size of this control. Call after changing one of the children */
void SV_NamedSlider::UpdateDimensions()
{
	Slider->AlignRightTo(Label, 0);

	SetSize(D2D1::SizeF(Label->GetSize().width + Slider->GetSize().width,
						Label->GetSize().height + Slider->GetSize().height));
}

/** Sets the position and size of this sub-view */
void SV_NamedSlider::SetRect(const D2D1_RECT_F& rect)
{
	D2DSubView::SetRect(rect);
}