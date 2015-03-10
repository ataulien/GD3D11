#pragma once
#include "pch.h"

class D3D11FullscreenQuad;
struct RenderToTextureBuffer;
class D3D11PFX_Blur;
class D3D11PFX_HeightFog;
class D3D11PFX_DistanceBlur;
class D3D11PFX_HDR;
class D3D11PFX_SMAA;
class D3D11NVHBAO;
class D3D11PfxRenderer
{
public:
	D3D11PfxRenderer(void);
	~D3D11PfxRenderer(void);

	/** Called on resize */
	XRESULT OnResize(const INT2& newResolution);

	/** Blurs the given texture */
	XRESULT BlurTexture(RenderToTextureBuffer* texture, bool leaveResultInD4_2 = false, float scale = 1.0f, const D3DXVECTOR4& colorMod = D3DXVECTOR4(1,1,1,1), const std::string& finalCopyShader = "PS_PFX_Simple");

	/** Renders the heightfog */
	XRESULT RenderHeightfog();

	/** Renders the distance blur effect */
	XRESULT RenderDistanceBlur();

	/** Renders the HDR-Effect */
	XRESULT RenderHDR();

	/** Renders the SMAA-Effect */
	XRESULT RenderSMAA();

	/** Copies the given texture to the given RTV */
	XRESULT CopyTextureToRTV(ID3D11ShaderResourceView* texture, ID3D11RenderTargetView* rtv, INT2 targetResolution = INT2(0,0), bool useCustomPS = false, INT2 offset = INT2(0,0));

	/** Unbinds texturesamplers from the pixel-shader */
	XRESULT UnbindPSResources(int num);

	/** Draws a fullscreenquad */
	XRESULT DrawFullScreenQuad();

	/** Draws the HBAO-Effect to the given buffer */
	XRESULT DrawHBAO(ID3D11RenderTargetView* rtv);

	/** Accessors */
	RenderToTextureBuffer* GetTempBuffer(){return TempBuffer;}
	RenderToTextureBuffer* GetTempBufferDS4_1(){return TempBufferDS4_1;}
	RenderToTextureBuffer* GetTempBufferDS4_2(){return TempBufferDS4_2;}

private:
	/** Temporary buffer in the same size/format as the backbuffer */
	RenderToTextureBuffer* TempBuffer;

	/** Temporary buffer with the resolution divided by 4 */
	RenderToTextureBuffer* TempBufferDS4_1;
	RenderToTextureBuffer* TempBufferDS4_2;

	/** Blur effect referenced here because it's often needed by PFX */
	D3D11PFX_Blur* FX_Blur;
	D3D11PFX_HeightFog* FX_HeightFog;
	D3D11PFX_DistanceBlur* FX_DistanceBlur;
	D3D11PFX_HDR* FX_HDR;
	D3D11PFX_SMAA* FX_SMAA;

	/** Fullscreen quad */
	D3D11FullscreenQuad* ScreenQuad;

	/** Nivida HBAO+ */
	D3D11NVHBAO* NvHBAO;
};

