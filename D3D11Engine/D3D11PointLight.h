#pragma once
#include "ShadowedPointLight.h"

struct VobLightInfo;
struct RenderToDepthStencilBuffer;
struct RenderToTextureBuffer;
struct VobInfo;
struct SkeletalVobInfo;
class BaseConstantBuffer;
class D3D11PointLight : public ShadowedPointLight
{
public:
	D3D11PointLight(VobLightInfo* info);
	~D3D11PointLight(void);

	/** Draws the surrounding scene into the cubemap */
	void RenderCubemap(bool forceUpdate = false);

	/** Binds the shadowmap to the pixelshader */
	void OnRenderLight();

	/** Debug-draws the cubemap to the screen */
	void DebugDrawCubeMap();

	/** Returns if this light needs an update */
	bool NeedsUpdate();

protected:
	/** Renders the scene with the given view-proj-matrices */
	void RenderCubemapFace(const D3DXMATRIX& view, const D3DXMATRIX& proj, UINT faceIdx);

	/** Renders all cubemap faces at once, using the geometry shader */
	void RenderFullCubemap();

	std::list<VobInfo*> VobCache;
	std::list<SkeletalVobInfo*> SkeletalVobCache;

	VobLightInfo* LightInfo;
	RenderToDepthStencilBuffer* DepthCubemap;
	D3DXMATRIX CubeMapViewMatrices[6];
	D3DXVECTOR3 LastUpdatePosition;
	BaseConstantBuffer* ViewMatricesCB;
};

