#pragma once
#include "d2dsubview.h"

class D3D11Texture;
class SV_Panel :
	public D2DSubView
{
public:
	SV_Panel(D2DView* view, D2DSubView* parent);
	~SV_Panel(void);

	enum EPanelRenderMode
	{
		PR_Background,
		PR_SolidColor,
		PR_Image
	};

	/** Sets the border color */
	void SetPanelColor(const D2D1_COLOR_F& color);

	/** Returns the panels solid color */
	D2D1_COLOR_F GetPanelColor();

	/** Set if this panel should have a shadow */
	void SetPanelShadow(bool shadow, float range = 20.0f);

	/** Set if this panel should render as a solid color */
	void SetRenderMode(EPanelRenderMode mode);

	/** Set if this should have a dark overlay on top */
	void SetDarkOverlay(bool value);

	/** Set if this should have a glossy outline */
	void SetGlossyOutline(bool value);

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Sets the image of this panel from d3d11 */
	HRESULT SetD3D11TextureAsImage(ID3D11Texture2D* texture, INT2 size);
protected:
	/** Border color */
	D2D1_COLOR_F PanelColor;

	/** Whether this has a shadow or not */
	bool HasShadow;
	float ShadowRange;

	/** What to render this as */
	EPanelRenderMode RenderMode;
	bool HasDarkOverlay;
	bool HasGlossyOutline;

	/** Image */
	ID2D1Bitmap* Image;
	ID3D11Texture2D* ImageSource;
};

