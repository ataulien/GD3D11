#include "pch.h"
#include "SV_Border.h"
#include "D2DView.h"

SV_Border::SV_Border(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	BorderColor = D2D1::ColorF(1,1,1,1);
}


SV_Border::~SV_Border(void)
{
}

/** Draws this sub-view */
void SV_Border::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

	MainView->GetBrush()->SetColor(BorderColor);
	MainView->GetRenderTarget()->DrawRectangle(ViewRect, MainView->GetBrush());

	D2DSubView::Draw(clientRectAbs, deltaTime);
}

/** Sets the border color */
void SV_Border::SetBorderColor(const D2D1_COLOR_F& color)
{
	BorderColor = color;
}