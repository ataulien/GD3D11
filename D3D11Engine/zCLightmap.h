#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"


class zCLightmap
{
public:

	D3DXVECTOR2 GetLightmapUV(const D3DXVECTOR3& worldPos)
	{
		D3DXVECTOR3 q = worldPos - LightmapOrigin;

		return D3DXVECTOR2( D3DXVec3Dot(&q, &LightmapUVRight),
							D3DXVec3Dot(&q, &LightmapUVUp) );
	}


	char data[0x24];

	D3DXVECTOR3	LightmapOrigin;
	D3DXVECTOR3	LightmapUVUp;
	D3DXVECTOR3	LightmapUVRight;

    zCTexture* Texture;
};