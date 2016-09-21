#pragma once
#include "pch.h"
#include "WorldConverter.h"
#include "GothicAPI.h"

/** Wrapper-class for some generic effects */

class D3D11VertexBuffer;
struct RenderToDepthStencilBuffer;
class D3D11Effect
{
public:
	D3D11Effect(void);
	~D3D11Effect(void);

	/** Draws GPU-Based rain */
	XRESULT DrawRain();

	/** Renders the rain-shadowmap */
	XRESULT DrawRainShadowmap();

	/** Returns the current rain-shadowmap camera replacement */
	CameraReplacement& GetRainShadowmapCameraRepl(){return RainShadowmapCameraRepl;}
	
	/** Returns the rain shadowmap */
	RenderToDepthStencilBuffer* GetRainShadowmap(){return RainShadowmap;}
protected:

	/** Fills a vector of random raindrop data */
	void FillRandomRaindropData(std::vector<ParticleInstanceInfo>& data);

	/** Rain */
	D3D11VertexBuffer* RainBufferInitial;
	D3D11VertexBuffer* RainBufferDrawFrom;
	D3D11VertexBuffer* RainBufferStreamTo;

	ID3D11Texture2D* RainTextureArray;
	ID3D11ShaderResourceView* RainTextureArraySRV;
	RenderToDepthStencilBuffer* RainShadowmap;
	CameraReplacement RainShadowmapCameraRepl;

};

