#pragma once
#include "d2dsubview.h"

class SV_Slider;
typedef void (__cdecl* SV_SliderValueChangedCallback)(SV_Slider*, void*);

class SV_Label;
class SV_Slider :
	public D2DSubView
{
public:
	SV_Slider(D2DView* view, D2DSubView* parent);
	~SV_Slider(void);

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Processes a window-message. Return false to stop the message from going to children */
	virtual bool OnWindowMessage(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs);

	/** Sets the value of this slider (0..1) */
	void SetValueP(float value);

	/** Sets the value of this slider (min..max) */
	void SetValue(float value);

	/** Sets the min-max values for this slider */
	void SetMinMax(float min, float max);

	/** Sets the data location to update with this slider */
	void SetDataToUpdate(float* data);
	void SetDataToUpdate(int* data);

	/** Sets whether this should display only ints or not */
	void SetIsIntegralSlider(bool value);

	/** Sets the callback */
	void SetSliderChangedCallback(SV_SliderValueChangedCallback fn, void* userdata);

	/** Returns the current value */
	float GetValue();

	/** Sets a value multiplier for displaying purposes */
	void SetDisplayMultiplier(float mul);

	/** Sets an array of values to display */
	void SetDisplayValues(const std::vector<std::string>& values);
protected:

	/** Draws the slider */
	void RenderSlider();

	/** Current bar position */
	float BarPosition;
	bool DraggingSlider;

	/** Slider values */
	float Value;
	float Min;
	float Max;
	float* DataToUpdate;
	int* DataToUpdateInt;
	SV_Label* ValueLabel;
	bool IsIntegral;
	std::vector<std::string> DisplayValues;
	float DisplayMultiplier;

	/** Callback */
	SV_SliderValueChangedCallback ValueChangedCallback;
	void* ValueChangedUserdata;

};

