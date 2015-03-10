#pragma once
#include "d3d11pfx_effect.h"
class D3D11PFX_Blur :
	public D3D11PFX_Effect
{
public:
	D3D11PFX_Blur(D3D11PfxRenderer* rnd);
	~D3D11PFX_Blur(void);

	/** Draws this effect to the given buffer */
	virtual XRESULT RenderBlur(RenderToTextureBuffer* fxbuffer, bool leaveResultInD4_2 = false, float threshold = 0.0f, float scale = 1.0f, const D3DXVECTOR4& colorMod = D3DXVECTOR4(1,1,1,1), const std::string& finalCopyShader = "PS_PFX_Simple");

	/** Draws this effect to the given buffer */
	virtual XRESULT Render(RenderToTextureBuffer* fxbuffer);
};

