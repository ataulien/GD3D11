#include "pch.h"
#include "GDecalVisual.h"
#include "zCDecal.h"
#include "zCVob.h"
#include "GVobObject.h"
#include "zCMaterial.h"
#include "BaseGraphicsEngine.h"
#include "Engine.h"
#include "BaseLineRenderer.h"


GDecalVisual::GDecalVisual(zCVisual* sourceVisual) : GVisual(sourceVisual)
{

}


GDecalVisual::~GDecalVisual(void)
{

}

/** Draws the visual for the given vob */
void GDecalVisual::DrawVisual(const RenderInfo& info)
{
#ifndef DEBUG_DRAW_VISUALS
	return;
#endif

	zCDecal* d = (zCDecal *)SourceVisual;

	// Get our view-matrix
	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	// Get alignment
	int alignment = info.CallingVob->GetSourceVob()->GetAlignment();

	D3DXMATRIX world = *info.WorldMatrix;
	
	// Decals can have a small offset in screenspace
	D3DXMATRIX offset; D3DXMatrixTranslation(&offset, d->GetDecalSettings()->DecalOffset.x,
		-d->GetDecalSettings()->DecalOffset.y, 0);

	// Make scale-matrix
	D3DXMATRIX scale; D3DXMatrixScaling(&scale, d->GetDecalSettings()->DecalSize.x * 2, -d->GetDecalSettings()->DecalSize.y * 2, 1);
	D3DXMatrixTranspose(&scale, &scale);

	D3DXMATRIX mat = view * world;

	if(alignment == zVISUAL_CAM_ALIGN_FULL || alignment == zVISUAL_CAM_ALIGN_YAW) 
	{
		for(int x=0;x<3;x++)
		{
			for(int y=0;y<3;y++) 
			{
				if ( x==y )
					mat(x,y) = 1.0;
				else
					mat(x,y) = 0.0;
			}
		}
	}

	mat = mat * offset * scale;

	// Bind texture
	if(d->GetDecalSettings()->DecalMaterial)
	{
		if(d->GetDecalSettings()->DecalMaterial->GetTexture())
		{
			if(d->GetDecalSettings()->DecalMaterial->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
			{
				return; // Don't render not cached surfaces
			}

			d->GetDecalSettings()->DecalMaterial->BindTexture(0);
		}
	}

	DrawDecalInstance(RenderInfo(info.CallingVob, &(world * offset * scale)));
}

/** Draws a single particle instance */
void GDecalVisual::DrawDecalInstance(const RenderInfo& info)
{
	D3DXVECTOR3 vx[6];

	float scale = 1.0f;
	vx[0] = D3DXVECTOR3(-scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[1] = D3DXVECTOR3(scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[2] = D3DXVECTOR3(-scale * 0.5f, scale * 0.5f, 0.0f);
		
	vx[3] = D3DXVECTOR3(scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[4] = D3DXVECTOR3(scale * 0.5f, scale * 0.5f, 0.0f);
	vx[5] = D3DXVECTOR3(-scale * 0.5f, scale * 0.5f, 0.0f);

	D3DXMATRIX w = *info.CallingVob->GetSourceVob()->GetWorldMatrixPtr();
	D3DXMatrixTranspose(&w, &w);

	zCDecal* d = (zCDecal *)SourceVisual;
	D3DXMATRIX scaleMat; D3DXMatrixScaling(&scaleMat, d->GetDecalSettings()->DecalSize.x * 2, -d->GetDecalSettings()->DecalSize.y * 2, 1);
	w = scaleMat * w;

	for(int i=0;i<6;i++)
	{
		// Transform to worldspace
		D3DXVec3TransformCoord(&vx[i], &vx[i], &w);
	}

	// Debug-draw decal
	for(int i=0;i<6;i+=3)
	{
		Engine::GraphicsEngine->GetLineRenderer()->AddTriangle(vx[i+0], vx[i+1], vx[i+2], D3DXVECTOR4(0,1,1,1));
	}
}
