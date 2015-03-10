#pragma once
#include "AntTweakBar.h"
#include "GothicAPI.h"

struct TS_TextureInfo
{
	TS_TextureInfo(MaterialInfo* info, const std::string& name, zCTexture* texture)
	{
		Info = info;
		Name = name;
		Texture = texture;
	}

	MaterialInfo* Info;
	std::string Name;
	zCTexture* Texture;
};

class BaseAntTweakBar
{
public:
	BaseAntTweakBar(void);
	virtual ~BaseAntTweakBar(void);

	/** Creates the resources */
	virtual XRESULT Init();

	/** On window message */
	virtual LRESULT OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/** Resizes the anttweakbar */
	virtual XRESULT OnResize(INT2 newRes);

	/** Sets the bars active and visible */
	virtual void SetActive(bool value);
	virtual bool GetActive();

	/** Draws the bars */
	virtual void Draw();

	/** Sets the preferred texture for the texture settings */
	void SetPreferredTextureForSettings(const std::string& texture);
protected:

	/** Updates the TS_Bar */
	void UpdateTextureSettingsBar();

	/** Initializes the TextureSettingsBar */
	void InitTextureSettingsBar();

	/** Whether the tweak-bar is active or not */
	bool IsActive;

	/** Called on "Apply"-Buttonpress */
	static void TW_CALL ReloadShadersButtonCallback(void *clientData);

	/** Called on load ZEN resources */
	static void TW_CALL LoadZENResourcesCallback(void* clientdata);

	/** Called on load ZEN resources */
	static void TW_CALL SaveZENResourcesCallback(void* clientdata);

	/** Called on load ZEN resources */
	static void TW_CALL OpenSettingsCallback(void* clientdata);

	/** Tweak bars */
	TwBar* Bar_Sky;

	TwBar* Bar_General;

	TwBar* Bar_Info;
	TwBar* Bar_HBAO;
	TwBar* Bar_ShaderMakros;

	std::string TS_PreferredTexture;
	TwBar* Bar_TextureSettings;
	std::string TS_TextureLastFrame;
	char TS_TextureName[256];
	MaterialInfo TS_OldMaterialInfo;
	bool TS_Active;
	int ActiveMaterialInfo;
	int LastFrameActiveMaterialInfo;
	std::vector<TS_TextureInfo> TS_FrameTexturesInfos;
};

