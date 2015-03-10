#pragma once
#include "d2ddialog.h"

class SV_GMeshInfoView;
class SV_NamedSlider;
class SV_Slider;
struct BaseVobInfo;
struct SkeletalVobInfo;
class D2DVobSettingsDialog :
	public D2DDialog
{
public:
	D2DVobSettingsDialog(D2DView* view, D2DSubView* parent);
	~D2DVobSettingsDialog(void);

	/** Initializes the controls of this view */
	virtual XRESULT InitControls();

	/** Sets the Vob to do settings on */
	void SetVobInfo(BaseVobInfo* vob);

private:
	/** Close button */
	static void CloseButtonPressed(SV_Button* sender, void* userdata);
	static void SliderDragged(SV_Slider* sender, void* userdata);

	/** Controls */
	SV_GMeshInfoView* MeshView;
	SV_NamedSlider* DisplacementStrengthSetting;
	SV_NamedSlider* TesselationFactorSetting;
	SV_NamedSlider* RoundnessSetting;
	SV_NamedSlider* RenderMode;

	/** Settings */
	BaseVobInfo* Vob;
};

