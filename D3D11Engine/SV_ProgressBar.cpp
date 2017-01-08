#include "pch.h"
#include "SV_ProgressBar.h"
#include "D2DView.h"

SV_ProgressBar::SV_ProgressBar(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	Progress = 0;
}


SV_ProgressBar::~SV_ProgressBar(void)
{
}


/** Draws this sub-view */
void SV_ProgressBar::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

	// Draw a shadow around the whole control
	MainView->DrawSmoothShadow(&ViewRect, 6, 0.5f, false, 2);

	// Draw the background box
	//float level=(float)Level*SF_DEF_LEVEL_COLOR_INC+SF_DEF_LEVEL_COLOR_BASE;

	float bgrColor = 0.6f;
	MainView->GetBrush()->SetColor(D2D1::ColorF(bgrColor,bgrColor,bgrColor,1));
	MainView->GetRenderTarget()->FillRectangle(&ViewRect, MainView->GetBrush());

	MainView->GetLinearReflectBrush()->SetStartPoint(D2D1::Point2F(-ViewRect.left,-ViewRect.top));
	MainView->GetLinearReflectBrush()->SetEndPoint(D2D1::Point2F(ViewRect.right-ViewRect.left,ViewRect.bottom-ViewRect.top));
	//SetLinearGradientToControl(D2DObjects->BrushCollection.LinearReflectBrush);

	D2D1_RECT_F sc = ViewRect;
	sc.right=(Progress*(sc.right-sc.left)) + sc.left;
	MainView->GetRenderTarget()->FillRectangle(&sc, MainView->GetLinearReflectBrush());

	D2DSubView::Draw(clientRectAbs, deltaTime);
}

/** Sets the progress of this bar */
void SV_ProgressBar::SetProgress(float p)
{
	Progress = p;
}