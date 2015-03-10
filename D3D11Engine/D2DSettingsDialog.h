#pragma once
#include "d2ddialog.h"
#include "GothicGraphicsState.h"
#include "BaseGraphicsEngine.h"

class SV_Slider;
class D2DSettingsDialog :
	public D2DDialog
{
public:
	D2DSettingsDialog(D2DView* view, D2DSubView* parent);
	~D2DSettingsDialog(void);

	/** Initializes the controls of this view */
	virtual XRESULT InitControls();

	/** Checks if a change needs to reload the shaders */
	bool NeedsApply();

protected:
	/** Tab in main tab-control was switched */
	static void ShadowQualitySliderChanged(SV_Slider* sender, void* userdata);
	static void ResolutionSliderChanged(SV_Slider* sender, void* userdata);

	/** Close button */
	static void CloseButtonPressed(SV_Button* sender, void* userdata);

	/** Apply button */
	static void ApplyButtonPressed(SV_Button* sender, void* userdata);

	/** Initial renderer settings, used to determine a change */
	GothicRendererSettings InitialSettings;

	/** Current resolution setting */
	int ResolutionSetting;
	std::vector<DisplayModeInfo> Resolutions;
};

