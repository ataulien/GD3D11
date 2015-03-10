#pragma once
#include "pch.h"

#pragma pack (push, 1)	
struct LineVertex
{
	LineVertex(const D3DXVECTOR3& position, DWORD color = 0xFFFFFFFF)
	{
		Position = position;
		Color = color;
	}

	LineVertex(const D3DXVECTOR3& position, const D3DXVECTOR4& color)
	{
		Position = position;
		Color = color;
	}

	float4 Position;
	float4 Color;
};
#pragma pack (pop)	

class BaseLineRenderer
{
public:
	BaseLineRenderer(void);
	virtual ~BaseLineRenderer(void);

	/** Adds a line to the list */
	virtual XRESULT AddLine(const LineVertex& v1, const LineVertex& v2) = 0;

	/** Flushes the cached lines */
	virtual XRESULT Flush() = 0;

	/** Clears the line cache */
	virtual XRESULT ClearCache() = 0;

	/** Adds a point locator to the renderlist */
	void AddPointLocator(const D3DXVECTOR3& location, float size=1, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1));

	/** Adds a plane to the renderlist */
	void AddPlane(const D3DXVECTOR4& plane, const D3DXVECTOR3& origin, float size=1, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1));

	/** Adds a ring to the renderlist */
	void AddRingZ(const D3DXVECTOR3& location, float size=1.0f, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1), int res=32);

	/** Adds an AABB-Box to the renderlist */
	void AddAABB(const D3DXVECTOR3& location, float halfSize, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1));
	void AddAABB(const D3DXVECTOR3& location, const D3DXVECTOR3& halfSize, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1));
	void AddAABBMinMax(const D3DXVECTOR3& min, const D3DXVECTOR3& max, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1));

	/** Adds a triangle to the renderlist */
	void AddTriangle(const D3DXVECTOR3& t0, const D3DXVECTOR3& t1, const D3DXVECTOR3& t2, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1));

	/** Plots a vector of floats */
	void PlotNumbers(const std::vector<float>& values, const D3DXVECTOR3& location, const D3DXVECTOR3& direction, float distance, float heightScale, const D3DXVECTOR4& color = D3DXVECTOR4(1,1,1,1));

};

