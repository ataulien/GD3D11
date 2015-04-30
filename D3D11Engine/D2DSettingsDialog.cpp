#include "pch.h"
#include "D2DSettingsDialog.h"
#include "D2DView.h"
#include "SV_Label.h"
#include "SV_Checkbox.h"
#include "SV_Panel.h"
#include "SV_Slider.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"

D2DSettingsDialog::D2DSettingsDialog(D2DView* view, D2DSubView* parent) : D2DDialog(view, parent)
{
	SetPositionCentered(D2D1::Point2F(view->GetRenderTarget()->GetSize().width / 2, view->GetRenderTarget()->GetSize().height / 2), D2D1::SizeF(500, 250));
	Header->SetCaption("Settings");

	// Get display modes
	Engine::GraphicsEngine->GetDisplayModeList(&Resolutions, false);

	// Find current
	ResolutionSetting = 0;
	for(unsigned int i=0;i<Resolutions.size();i++)
	{
		if( Resolutions[i].Width == Engine::GraphicsEngine->GetResolution().x &&
			Resolutions[i].Height == Engine::GraphicsEngine->GetResolution().y)
		{
			ResolutionSetting = i;
			break;
		}
	}

	InitControls();

	
}


D2DSettingsDialog::~D2DSettingsDialog(void)
{
}

/** Initializes the controls of this view */
XRESULT D2DSettingsDialog::InitControls()
{
	D2DSubView::InitControls();

	AddButton("Close", CloseButtonPressed, this);
	AddButton("[*] Apply", ApplyButtonPressed, this);

	SV_Label* resolutionLabel = new SV_Label(MainView, MainPanel);
	resolutionLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	resolutionLabel->AlignUnder(Header, 10);
	resolutionLabel->SetCaption("Resolution [*]:");
	resolutionLabel->SetPosition(D2D1::Point2F(5, resolutionLabel->GetPosition().y));

	SV_Slider* resolutionSlider = new SV_Slider(MainView, MainPanel);
	resolutionSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	resolutionSlider->AlignUnder(resolutionLabel, 5);
	resolutionSlider->SetSliderChangedCallback(ResolutionSliderChanged, this);
	resolutionSlider->SetIsIntegralSlider(true);
	resolutionSlider->SetMinMax(0.0f, (float)Resolutions.size());

	// Construct string array for resolutions slider
	std::vector<std::string> resStrings;
	for(unsigned int i=0;i<Resolutions.size();i++)
	{
		std::string str = std::to_string(Resolutions[i].Width) + "x" + std::to_string(Resolutions[i].Height);
		resStrings.push_back(str);
	}
	resolutionSlider->SetDisplayValues(resStrings);
	resolutionSlider->SetValue((float)ResolutionSetting);

	SV_Checkbox* hbaoCheckbox = new SV_Checkbox(MainView, MainPanel);
	hbaoCheckbox->SetSize(D2D1::SizeF(160, 20));
	hbaoCheckbox->SetCaption("Enable HBAO+");
	hbaoCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.Enabled);
	hbaoCheckbox->AlignUnder(resolutionSlider, 10);
	hbaoCheckbox->SetPosition(D2D1::Point2F(5, hbaoCheckbox->GetPosition().y));
	hbaoCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.Enabled);

	SV_Checkbox* godraysCheckbox = new SV_Checkbox(MainView, MainPanel);
	godraysCheckbox->SetSize(D2D1::SizeF(160, 20));
	godraysCheckbox->SetCaption("Enable GodRays");
	godraysCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableGodRays);
	godraysCheckbox->AlignUnder(hbaoCheckbox, 5);
	godraysCheckbox->SetPosition(D2D1::Point2F(5, godraysCheckbox->GetPosition().y));
	godraysCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableGodRays);

	SV_Checkbox* smaaCheckbox = new SV_Checkbox(MainView, MainPanel);
	smaaCheckbox->SetSize(D2D1::SizeF(160, 20));
	smaaCheckbox->SetCaption("Enable SMAA");
	smaaCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableSMAA);
	smaaCheckbox->AlignUnder(godraysCheckbox, 5);
	smaaCheckbox->SetPosition(D2D1::Point2F(5, smaaCheckbox->GetPosition().y));
	smaaCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableSMAA);

	SV_Checkbox* tesselationCheckbox = new SV_Checkbox(MainView, MainPanel);
	tesselationCheckbox->SetSize(D2D1::SizeF(160, 20));
	tesselationCheckbox->SetCaption("Enable Tesselation");
	tesselationCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableTesselation);
	tesselationCheckbox->AlignUnder(smaaCheckbox, 5);
	tesselationCheckbox->SetPosition(D2D1::Point2F(5, tesselationCheckbox->GetPosition().y));
	tesselationCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableTesselation);

	

	SV_Checkbox* shadowsCheckbox = new SV_Checkbox(MainView, MainPanel);
	shadowsCheckbox->SetSize(D2D1::SizeF(160, 20));
	shadowsCheckbox->SetCaption("Enable Shadows[*]");
	shadowsCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableShadows);
	shadowsCheckbox->AlignUnder(tesselationCheckbox, 5);
	shadowsCheckbox->SetPosition(D2D1::Point2F(5, shadowsCheckbox->GetPosition().y));
	shadowsCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableShadows);

	SV_Checkbox* filterShadowsCheckbox = new SV_Checkbox(MainView, MainPanel);
	filterShadowsCheckbox->SetSize(D2D1::SizeF(160, 20));
	filterShadowsCheckbox->SetCaption("Shadow Filtering [*]");
	filterShadowsCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableSoftShadows);
	filterShadowsCheckbox->AlignUnder(shadowsCheckbox, 5);
	filterShadowsCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableSoftShadows);

	SV_Label* shadowmapSizeLabel = new SV_Label(MainView, MainPanel);
	shadowmapSizeLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	shadowmapSizeLabel->AlignUnder(filterShadowsCheckbox, 10);
	shadowmapSizeLabel->SetCaption("Shadow Quality:");

	SV_Slider* shadowmapSizeSlider = new SV_Slider(MainView, MainPanel);
	shadowmapSizeSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	shadowmapSizeSlider->AlignUnder(shadowmapSizeLabel, 5);
	shadowmapSizeSlider->SetSliderChangedCallback(ShadowQualitySliderChanged, this);
	shadowmapSizeSlider->SetIsIntegralSlider(true);
	shadowmapSizeSlider->SetMinMax(1.0f, 4.0f);

	// Fix the shadow range
	switch(Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize)
	{
	case 512:
		shadowmapSizeSlider->SetValue(1);
		break;

	case 1024:
		shadowmapSizeSlider->SetValue(2);
		break;

	case 2048:
		shadowmapSizeSlider->SetValue(3);
		break;

	case 4096:
		shadowmapSizeSlider->SetValue(4);
		break;
	}


	// Next column
	/*SV_Checkbox* hdrCheckbox = new SV_Checkbox(MainView, MainPanel);
	hdrCheckbox->SetSize(D2D1::SizeF(160, 20));
	hdrCheckbox->SetCaption("Enable HDR");
	hdrCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableHDR);
	hdrCheckbox->AlignUnder(Header, 5);
	hdrCheckbox->SetPosition(D2D1::Point2F(170, hdrCheckbox->GetPosition().y));
	hdrCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableHDR);*/

	SV_Checkbox* vsyncCheckbox = new SV_Checkbox(MainView, MainPanel);
	vsyncCheckbox->SetSize(D2D1::SizeF(160, 20));
	vsyncCheckbox->SetCaption("Enable VSync");
	vsyncCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableVSync);
	vsyncCheckbox->AlignUnder(Header, 5);
	vsyncCheckbox->SetPosition(D2D1::Point2F(170, vsyncCheckbox->GetPosition().y));
	vsyncCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableVSync);

	SV_Label* outdoorVobsDDLabel = new SV_Label(MainView, MainPanel);
	outdoorVobsDDLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	outdoorVobsDDLabel->AlignUnder(vsyncCheckbox, 10);
	outdoorVobsDDLabel->SetCaption("Object draw distance:");

	SV_Slider* outdoorVobsDDSlider = new SV_Slider(MainView, MainPanel);
	outdoorVobsDDSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	outdoorVobsDDSlider->AlignUnder(outdoorVobsDDLabel, 5);
	outdoorVobsDDSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius);
	outdoorVobsDDSlider->SetIsIntegralSlider(true);
	outdoorVobsDDSlider->SetDisplayMultiplier(0.001f);
	outdoorVobsDDSlider->SetMinMax(0.0f, 99999.0f);
	outdoorVobsDDSlider->SetValue(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius);

	SV_Label* outdoorVobsSmallDDLabel = new SV_Label(MainView, MainPanel);
	outdoorVobsSmallDDLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	outdoorVobsSmallDDLabel->AlignUnder(outdoorVobsDDSlider, 8);
	outdoorVobsSmallDDLabel->SetCaption("Small object draw distance:");

	SV_Slider* outdoorVobsSmallDDSlider = new SV_Slider(MainView, MainPanel);
	outdoorVobsSmallDDSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	outdoorVobsSmallDDSlider->AlignUnder(outdoorVobsSmallDDLabel, 5);
	outdoorVobsSmallDDSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius);
	outdoorVobsSmallDDSlider->SetIsIntegralSlider(true);
	outdoorVobsSmallDDSlider->SetDisplayMultiplier(0.001f);
	outdoorVobsSmallDDSlider->SetMinMax(0.0f, 99999.0f);
	outdoorVobsSmallDDSlider->SetValue(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius);

	SV_Label* visualFXDDLabel = new SV_Label(MainView, MainPanel);
	visualFXDDLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	visualFXDDLabel->AlignUnder(outdoorVobsSmallDDSlider, 8);
	visualFXDDLabel->SetCaption("VisualFX draw distance:");

	SV_Slider* visualFXDDSlider = new SV_Slider(MainView, MainPanel);
	visualFXDDSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	visualFXDDSlider->AlignUnder(visualFXDDLabel, 5);
	visualFXDDSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius);
	visualFXDDSlider->SetIsIntegralSlider(true);
	visualFXDDSlider->SetDisplayMultiplier(0.001f);
	visualFXDDSlider->SetMinMax(0.0f, 10000.0f);
	visualFXDDSlider->SetValue(Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius);

	SV_Label* worldDDLabel = new SV_Label(MainView, MainPanel);
	worldDDLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	worldDDLabel->AlignUnder(visualFXDDSlider, 8);
	worldDDLabel->SetCaption("World draw distance:");

	SV_Slider* worldDDSlider = new SV_Slider(MainView, MainPanel);
	worldDDSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	worldDDSlider->AlignUnder(worldDDLabel, 5);
	worldDDSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius);
	worldDDSlider->SetIsIntegralSlider(true);
	worldDDSlider->SetMinMax(1.0f, 10.0f);
	worldDDSlider->SetValue((float)Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius);


	// Third column
	// FOV
	SV_Label* horizFOVLabel = new SV_Label(MainView, MainPanel);
	horizFOVLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	//horizFOVLabel->AlignUnder(outdoorVobsSmallDDSlider, 10);
	horizFOVLabel->SetCaption("Horizontal FOV:");
	horizFOVLabel->AlignUnder(Header, 5);
	horizFOVLabel->SetPosition(D2D1::Point2F(170 + 160 + 10, horizFOVLabel->GetPosition().y));

	SV_Slider* horizFOVSlider = new SV_Slider(MainView, MainPanel);
	horizFOVSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	horizFOVSlider->AlignUnder(horizFOVLabel, 5);
	horizFOVSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.FOVHoriz);
	horizFOVSlider->SetIsIntegralSlider(true);
	horizFOVSlider->SetMinMax(40.0f, 150.0f);
	horizFOVSlider->SetValue(Engine::GAPI->GetRendererState()->RendererSettings.FOVHoriz);

	SV_Label* vertFOVLabel = new SV_Label(MainView, MainPanel);
	vertFOVLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	vertFOVLabel->AlignUnder(horizFOVSlider, 10);
	vertFOVLabel->SetCaption("Vertical FOV:");

	SV_Slider* vertFOVSlider = new SV_Slider(MainView, MainPanel);
	vertFOVSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	vertFOVSlider->AlignUnder(vertFOVLabel, 5);
	vertFOVSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.FOVVert);
	vertFOVSlider->SetIsIntegralSlider(true);
	vertFOVSlider->SetMinMax(40.0f, 150.0f);
	vertFOVSlider->SetValue(Engine::GAPI->GetRendererState()->RendererSettings.FOVVert);

	SV_Label* brightnessLabel = new SV_Label(MainView, MainPanel);
	brightnessLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	brightnessLabel->AlignUnder(vertFOVSlider, 10);
	brightnessLabel->SetCaption("Brightness:");

	SV_Slider* brightnessSlider = new SV_Slider(MainView, MainPanel);
	brightnessSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	brightnessSlider->AlignUnder(brightnessLabel, 5);
	brightnessSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.BrightnessValue);
	brightnessSlider->SetMinMax(0.1f, 3.0f);
	brightnessSlider->SetValue(Engine::GAPI->GetRendererState()->RendererSettings.BrightnessValue);

	SV_Label* contrastLabel = new SV_Label(MainView, MainPanel);
	contrastLabel->SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(150, 12));
	contrastLabel->AlignUnder(brightnessSlider, 10);
	contrastLabel->SetCaption("Contrast:");

	SV_Slider* contrastSlider = new SV_Slider(MainView, MainPanel);
	contrastSlider->SetPositionAndSize(D2D1::Point2F(10, 22), D2D1::SizeF(150, 15));
	contrastSlider->AlignUnder(contrastLabel, 5);
	contrastSlider->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.GammaValue);
	contrastSlider->SetMinMax(0.1f, 2.0f);
	contrastSlider->SetValue(Engine::GAPI->GetRendererState()->RendererSettings.GammaValue);


	SV_Checkbox* updateCheckbox = new SV_Checkbox(MainView, MainPanel);
	updateCheckbox->SetSize(D2D1::SizeF(160, 20));
	updateCheckbox->SetCaption("Enable Autoupdates");
	updateCheckbox->SetDataToUpdate(&Engine::GAPI->GetRendererState()->RendererSettings.EnableAutoupdates);
	updateCheckbox->AlignUnder(contrastSlider, 10);
	updateCheckbox->SetChecked(Engine::GAPI->GetRendererState()->RendererSettings.EnableAutoupdates);



	// Advanced settings label
	SV_Label* advancedSettingsLabel = new SV_Label(MainView, MainPanel);
	advancedSettingsLabel->SetPositionAndSize(D2D1::Point2F(5, GetSize().height - 5 - 12), D2D1::SizeF(200, 12));
	advancedSettingsLabel->SetCaption("CTRL+F11 -> Advanced");

	return XR_SUCCESS;
}

/** Tab in main tab-control was switched */
void D2DSettingsDialog::ShadowQualitySliderChanged(SV_Slider* sender, void* userdata)
{
	switch((int)(sender->GetValue() + 0.5f))
	{
	case 1:
		Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize = 512;
		break;

	case 2:
		Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize = 1024;
		break;

	case 3:
		Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize = 2048;
		break;

	case 4:
		Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize = 4096;
		break;
	}
}

void D2DSettingsDialog::ResolutionSliderChanged(SV_Slider* sender, void* userdata)
{
	D2DSettingsDialog* d = (D2DSettingsDialog *)userdata;

	unsigned int val = (unsigned int)(sender->GetValue() + 0.5f);

	if(val >= d->Resolutions.size())
		val = d->Resolutions.size()-1;

	d->ResolutionSetting = val;

}

/** Close button */
void D2DSettingsDialog::CloseButtonPressed(SV_Button* sender, void* userdata)
{
	D2DSettingsDialog* d = (D2DSettingsDialog *)userdata;

	if(d->NeedsApply())
		d->ApplyButtonPressed(sender, userdata);

	d->SetHidden(true);

	Engine::GAPI->SaveMenuSettings(MENU_SETTINGS_FILE);
	Engine::GAPI->SetEnableGothicInput(true);
}

/** Apply button */
void D2DSettingsDialog::ApplyButtonPressed(SV_Button* sender, void* userdata)
{
	GothicRendererSettings& settings = Engine::GAPI->GetRendererState()->RendererSettings;
	D2DSettingsDialog* d = (D2DSettingsDialog *)userdata;

	// Check for shader reload
	if( d->InitialSettings.EnableShadows != settings.EnableShadows ||
		d->InitialSettings.EnableSoftShadows != settings.EnableSoftShadows)
	{
		Engine::GraphicsEngine->ReloadShaders();
	}

	// Check for resolution change
	if( d->Resolutions[d->ResolutionSetting].Width != Engine::GraphicsEngine->GetResolution().x ||
		d->Resolutions[d->ResolutionSetting].Height != Engine::GraphicsEngine->GetResolution().y)
	{
		Engine::GraphicsEngine->OnResize(INT2(d->Resolutions[d->ResolutionSetting].Width, d->Resolutions[d->ResolutionSetting].Height));
	}
}

/** Checks if a change needs to reload the shaders */
bool D2DSettingsDialog::NeedsApply()
{
	GothicRendererSettings& settings = Engine::GAPI->GetRendererState()->RendererSettings;

	// Check for shader reload
	if( InitialSettings.EnableShadows != settings.EnableShadows ||
		InitialSettings.EnableSoftShadows != settings.EnableSoftShadows)
	{
		return true;
	}

	// Check for resolution change
	if( Resolutions[ResolutionSetting].Width != Engine::GraphicsEngine->GetResolution().x ||
			Resolutions[ResolutionSetting].Height != Engine::GraphicsEngine->GetResolution().y)
	{
		return true;
	}

	return false;
}

/** Called when the settings got re-opened */
void D2DSettingsDialog::OnOpenedSettings()
{
	InitialSettings = Engine::GAPI->GetRendererState()->RendererSettings;
}

/** Sets if this control is hidden */
void D2DSettingsDialog::SetHidden(bool hidden)
{
	if(IsHidden() && !hidden)
		OnOpenedSettings(); // Changed visibility from hidden to non-hidden

	D2DDialog::SetHidden(hidden);
}