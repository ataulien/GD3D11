#include "pch.h"
#include "SV_Slider.h"
#include "D2DSubView.h"
#include "D2DView.h"
#include "SV_Label.h"

const float SV_SLIDERCONTROL_SLIDER_SIZEX = 5;

SV_Slider::SV_Slider(D2DView* view, D2DSubView* parent) : D2DSubView(view, parent)
{
	BarPosition = 0.0f;
	Min = 0.0f;
	Max = 1.0f;
	Value = 0.0f;
	DisplayMultiplier = 1.0f;
	DraggingSlider = false;
	IsIntegral = false;

	DataToUpdate = NULL;
	DataToUpdateInt = NULL;

	// Add label
	ValueLabel = new SV_Label(view, this);
	ValueLabel->SetHorizAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	ValueLabel->SetVertAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	ValueLabel->SetTextSize(9);
	ValueLabel->SetTextColor(D2D1::ColorF(1,1,1,0.6f));

	SetSliderChangedCallback(NULL, NULL);
}


SV_Slider::~SV_Slider(void)
{
}

/** Processes a window-message. Return false to stop the message from going to children */
bool SV_Slider::OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs)
{
	switch(msg)
	{


	case WM_LBUTTONDOWN:
		{
			POINT p = D2DView::GetCursorPosition();
			if(PointInsideRect(D2D1::Point2F((float)p.x, (float)p.y), clientRectAbs))
			{
				DraggingSlider = true;
				return false;
			}
		}
		// Let this run into WM_MOUSEMOVE, so we get an update immediately after clicking
		// There is no break here!

	case WM_MOUSEMOVE:
		if(DraggingSlider)
		{
			// Calculate valueP from the x-axis
			POINT p = D2DView::GetCursorPosition();

			float left = clientRectAbs.left + SV_SLIDERCONTROL_SLIDER_SIZEX * 2;
			float right = clientRectAbs.right - SV_SLIDERCONTROL_SLIDER_SIZEX * 2;
			float vP = (p.x - left) / (right - left);
			SetValueP(vP);
			return false;
		}
		break;

	case WM_LBUTTONUP:
		if(DraggingSlider)
		{
			DraggingSlider = false;
			return false;
		}
		break;
	}

	return D2DSubView::OnWindowMessage(hWnd, msg, wParam, lParam, clientRectAbs);
}

/** Draws this sub-view */
void SV_Slider::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	//Set up the layer for this control
	MainView->GetRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(clientRectAbs.left,clientRectAbs.top));

	// Draw a shadow around the whole control
	//MainView->DrawSmoothShadow(&ViewRect, 10, 0.5f, false, 7);
	MainView->DrawSmoothShadow(&ViewRect, 15.0f, 0.8f, false, 7);

	RenderSlider();
	/*
	// Draw the background box
	//float Level=(float)GetLevel()*SF_DEF_LEVEL_COLOR_INC+SF_DEF_LEVEL_COLOR_BASE;
	MainView->GetBrush()->SetColor(D2D1::ColorF(0.2f,0.2f,0.2f,1));
	MainView->GetRenderTarget()->FillRectangle(&ViewRect, MainView->GetBrush());
	MainView->DrawSmoothShadow(&ViewRect, -5/2, 0.5f, 0.0f);


	// Draw the outer lines
	MainView->GetLinearReflectBrush()->SetStartPoint(D2D1::Point2F(ViewRect.left, ViewRect.top));
	MainView->GetLinearReflectBrush()->SetEndPoint(D2D1::Point2F(ViewRect.right, ViewRect.bottom));
	MainView->GetRenderTarget()->DrawRectangle(&ViewRect, MainView->GetLinearReflectBrush());

	D2D1_RECT_F sc = ViewRect;
	D2DView::ShrinkRect(&sc, 1);
	MainView->GetBrush()->SetColor( SV_DEF_INNER_LINE_COLOR );
	MainView->GetRenderTarget()->DrawRectangle(&sc, MainView->GetBrush());
	
	sc = ViewRect;

	// Draw slider
	//SetLinearGradientToControl(D2DObjects->BrushCollection.LinearReflectBrush);
	MainView->GetBrush()->SetColor(D2D1::ColorF(0.2f,0.2f,0.2f,1));
	sc.right = BarPosition + sc.left + SV_SLIDERCONTROL_SLIDER_SIZEX;
	sc.left = sc.right - (SV_SLIDERCONTROL_SLIDER_SIZEX*2);
	sc.top+=2;
	sc.bottom-=2;
	MainView->GetRenderTarget()->FillRectangle(&sc, MainView->GetBrush());

	// Draw outer lines of the slider
	MainView->GetRenderTarget()->DrawRectangle(&sc, MainView->GetLinearReflectBrush());
	D2DView::ShrinkRect(&sc, 1);
	MainView->GetBrush()->SetColor( SV_DEF_INNER_LINE_COLOR );
	MainView->GetRenderTarget()->DrawRectangle(&sc, MainView->GetBrush());
	*/
	D2DSubView::Draw(clientRectAbs, deltaTime);
}

void SV_Slider::RenderSlider()
{
	//First draw a box from the top to the bottom
	D2D1_RECT_F sc = ViewRect;
	D2D1_RECT_F bc = ViewRect;

	D2D1_COLOR_F BarBackgroundColor = D2D1::ColorF(0.2f,0.2f,0.2f,1.0f);
	float BarInnerShadowStrength = 0.8f;

	MainView->GetBrush()->SetColor(BarBackgroundColor);
	MainView->GetRenderTarget()->FillRectangle(sc,MainView->GetBrush());

	MainView->DrawSmoothShadow(&sc, -SV_DEF_SHADOW_RANGE, BarInnerShadowStrength);
	
	MainView->GetLinearReflectBrushHigh()->SetStartPoint(D2D1::Point2F(ViewRect.left, ViewRect.top));
	MainView->GetLinearReflectBrushHigh()->SetEndPoint(D2D1::Point2F(ViewRect.right, ViewRect.bottom));
	MainView->GetRenderTarget()->DrawRectangle(sc, MainView->GetLinearReflectBrushHigh());

	MainView->GetBrush()->SetColor( SV_DEF_INNER_LINE_COLOR );
	D2DView::ShrinkRect(&sc,1);

	MainView->GetRenderTarget()->DrawRectangle(sc, MainView->GetBrush());
	sc = ViewRect;

	MainView->GetBrush()->SetColor( BarBackgroundColor );
	
	bc.right = BarPosition + bc.left + SV_SLIDERCONTROL_SLIDER_SIZEX;
	bc.left = bc.right - (SV_SLIDERCONTROL_SLIDER_SIZEX*2);
	bc.top+=2;
	bc.bottom-=2;
	
	//SetLinearGradientToRect(D2DObjects->BrushCollection.BlueGradientBrush,&bc,true);
	//MainView->GetLinearReflectBrushHigh()->SetStartPoint(D2D1::Point2F(ViewRect.left, ViewRect.top));
	//MainView->GetLinearReflectBrushHigh()->SetEndPoint(D2D1::Point2F(ViewRect.right, ViewRect.bottom));
	MainView->GetRenderTarget()->FillRectangle(bc,MainView->GetBrush());
	
	MainView->GetLinearReflectBrush()->SetStartPoint(D2D1::Point2F(ViewRect.left, ViewRect.top));
	MainView->GetLinearReflectBrush()->SetEndPoint(D2D1::Point2F(ViewRect.right, ViewRect.bottom));
	MainView->GetRenderTarget()->DrawRectangle(bc, MainView->GetLinearReflectBrush());

	MainView->GetBrush()->SetColor( D2D1::ColorF(0,0,0, 0.5) );
	D2DView::ShrinkRect(&bc,1);
	MainView->GetRenderTarget()->DrawRectangle(bc,MainView->GetBrush(), 1);

	// If bar position on the left side, place label to the right of the slider
	if(BarPosition < GetSize().width / 2)
	{
		ValueLabel->SetPosition(D2D1::Point2F(BarPosition + 2 + SV_SLIDERCONTROL_SLIDER_SIZEX,0));
		ValueLabel->SetSize(GetSize());
		ValueLabel->SetHorizAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	}else
	{
		ValueLabel->SetPosition(D2D1::Point2F(0,0));
		ValueLabel->SetSize(D2D1::SizeF((BarPosition - 2 - SV_SLIDERCONTROL_SLIDER_SIZEX), GetSize().height));
		ValueLabel->SetHorizAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	}

	// Only change when needed
	if((DraggingSlider && (!DataToUpdate)) || (DataToUpdate && *DataToUpdate != Value) || !ValueLabel)
	{
		if(DisplayValues.empty())
		{
			// Remove trailing zeros
			std::string str;
			char txt[32];

			if(IsIntegral)
			{
				sprintf_s(txt, "%d", (int)(Value * DisplayMultiplier + 0.5f));
				str = txt;
			}else
			{
				sprintf_s(txt, "%.2f", Value * DisplayMultiplier);
				str = txt;

				str.erase(str.find_last_not_of('0') + 1, std::string::npos);

				// Fix integral numbers going like "100.", make them go "100" instead"
				if(str.back() == '.')
					str.pop_back();
			}

			ValueLabel->SetCaption(str);
		}else
		{
			unsigned int v = (unsigned int)(Value + 0.5f);
			if(v >= DisplayValues.size())
				v = DisplayValues.size() - 1;

			ValueLabel->SetCaption(DisplayValues[v]);
		}
	}

	/*
	//SetLinearGradientToRect(D2DObjects->BrushCollection.LinearReflectBrushHigh,&bc,true);
	D2DObjects->RenderTarget->FillRectangle(bc,D2DObjects->RenderBrush);
	*/

	/*bc.left+=SF_SCROLLBAR_OFFSET;
	bc.right-=SF_SCROLLBAR_OFFSET;
	bc.top+=BarPosition-(BarSize/2);
	bc.bottom=sc.top+BarPosition+(BarSize/2);*/

	
}

/** Sets the value of this slider (0..1) */
void SV_Slider::SetValueP(float value)
{
	value = std::min(1.0f, value);
	value = std::max(0.0f, value);

	float oldVal = Value;
	Value = Toolbox::lerp(Min, Max, value);

	if(IsIntegral)
		Value = (float)((int)(Value + 0.5f));

	BarPosition = (GetSize().width - ((SV_SLIDERCONTROL_SLIDER_SIZEX+2) * 2)) * value + (SV_SLIDERCONTROL_SLIDER_SIZEX+2);

	// Update data
	if(DataToUpdate)
		*DataToUpdate = Value;

	if(DataToUpdateInt)
		*DataToUpdateInt = (int)(Value + 0.5f);

	// Fire callback
	if(oldVal != Value && ValueChangedCallback)
		ValueChangedCallback(this, ValueChangedUserdata);

	// Update caption:
	if(DisplayValues.empty())
	{
		// Remove trailing zeros
		std::string str;
		char txt[32];

		if(IsIntegral)
		{
			sprintf_s(txt, "%d", (int)(Value * DisplayMultiplier + 0.5f));
			str = txt;
		}else
		{
			sprintf_s(txt, "%.2f", Value * DisplayMultiplier);
			str = txt;

			str.erase(str.find_last_not_of('0') + 1, std::string::npos);

			// Fix integral numbers going like "100.", make them go "100" instead"
			if(str.back() == '.')
				str.pop_back();
		}

		ValueLabel->SetCaption(str);
	}else
	{
		unsigned int v = (unsigned int)(Value + 0.5f);
		if(v >= DisplayValues.size())
			v = DisplayValues.size() - 1;

		ValueLabel->SetCaption(DisplayValues[v]);
	}
}

/** Sets the value of this slider (min..max) */
void SV_Slider::SetValue(float value)
{
	SetValueP((value - Min) / (Max - Min));
}

/** Sets the min-max values for this slider */
void SV_Slider::SetMinMax(float min, float max)
{
	Min = min;
	Max = max;
}

/** Sets the data location to update with this slider */
void SV_Slider::SetDataToUpdate(float* data)
{
	DataToUpdate = data;
}

/** Sets the data location to update with this slider */
void SV_Slider::SetDataToUpdate(int* data)
{
	DataToUpdateInt = data;
}

/** Sets the callback */
void SV_Slider::SetSliderChangedCallback(SV_SliderValueChangedCallback fn, void* userdata)
{
	ValueChangedUserdata = userdata;
	ValueChangedCallback = fn;
}

/** Returns the current value */
float SV_Slider::GetValue()
{
	return IsIntegral ? (int)(Value + 0.5f) : Value;
}

/** Sets whether this should display only ints or not */
void SV_Slider::SetIsIntegralSlider(bool value)
{
	IsIntegral = value;
}

/** Sets an array of values to display */
void SV_Slider::SetDisplayValues(const std::vector<std::string>& values)
{
	DisplayValues = values;
}

/** Sets a value multiplier for displaying purposes */
void SV_Slider::SetDisplayMultiplier(float mul)
{
	DisplayMultiplier = mul;
}