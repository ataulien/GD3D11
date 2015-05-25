#include "pch.h"
#include "BaseLineRenderer.h"


BaseLineRenderer::BaseLineRenderer(void)
{
}


BaseLineRenderer::~BaseLineRenderer(void)
{
}

/** Plots a vector of floats */
void BaseLineRenderer::PlotNumbers(const std::vector<float>& values, const D3DXVECTOR3& location, const D3DXVECTOR3& direction, float distance, float heightScale, const D3DXVECTOR4& color)
{
	for(unsigned int i=1;i<values.size();i++)
	{
		AddLine(LineVertex(location + (direction * (float)(i-1) * distance) + D3DXVECTOR3(0,0,values[i-1]*heightScale), color), 
				LineVertex(location + (direction * (float)i * distance) + D3DXVECTOR3(0,0,values[i]*heightScale), color));
	}
}

/** Adds a triangle to the renderlist */
void BaseLineRenderer::AddTriangle(const D3DXVECTOR3& t0, const D3DXVECTOR3& t1, const D3DXVECTOR3& t2, const D3DXVECTOR4& color)
{
	AddLine(LineVertex(t0, color), LineVertex(t1, color));
	AddLine(LineVertex(t0, color), LineVertex(t2, color));
	AddLine(LineVertex(t1, color), LineVertex(t2, color));
}

/** Adds a point locator to the renderlist */
void BaseLineRenderer::AddPointLocator(const D3DXVECTOR3& location, float size, const D3DXVECTOR4& color)
{
	D3DXVECTOR3 u = location; u.z += size;
	D3DXVECTOR3 d = location; d.z -= size;

	D3DXVECTOR3 r = location; r.x += size;
	D3DXVECTOR3 l = location; l.x -= size;

	D3DXVECTOR3 b = location; b.y += size;
	D3DXVECTOR3 f = location; f.y -= size;

	AddLine(LineVertex(u, color), LineVertex(d, color));
	AddLine(LineVertex(r, color), LineVertex(l, color));
	AddLine(LineVertex(f, color), LineVertex(b, color));
}

/** Adds a plane to the renderlist */
void BaseLineRenderer::AddPlane(const D3DXVECTOR4& plane, const D3DXVECTOR3& origin, float size, const D3DXVECTOR4& color)
{
	D3DXVECTOR3 pNormal = D3DXVECTOR3(plane);

	D3DXVECTOR3 DebugPlaneP1;
	DebugPlaneP1.x = 1;
	DebugPlaneP1.y = 1;
	DebugPlaneP1.z = ((-plane.x - plane.y) / plane.z);

	D3DXVec3Normalize(&DebugPlaneP1,&DebugPlaneP1);

	D3DXVECTOR3 DebugPlaneP2;
	D3DXVec3Cross(&DebugPlaneP2, &pNormal, &DebugPlaneP1);

	//DebugPlaneP2 += SlidingPlaneOrigin;
	D3DXVECTOR3& p1 = DebugPlaneP1;
	D3DXVECTOR3& p2 = DebugPlaneP2;
	D3DXVECTOR3 O = origin;

	D3DXVECTOR3 from; D3DXVECTOR3 to;
	from = (O - p1) - p2; 
	to = (O - p1) + p2; 
	AddLine(LineVertex(from), LineVertex(to));

	from = (O - p1) + p2; 
	to = (O + p1) + p2; 
	AddLine(LineVertex(from), LineVertex(to));

	from = (O + p1) + p2; 
	to = (O + p1) - p2; 
	AddLine(LineVertex(from), LineVertex(to));

	from = (O + p1) - p2; 
	to = (O - p1) - p2; 
	AddLine(LineVertex(from), LineVertex(to));
}

/** Adds an AABB-Box to the renderlist */
void BaseLineRenderer::AddAABB(const D3DXVECTOR3& location, float halfSize, const D3DXVECTOR4& color)
{
	// Bottom -x -y -z to +x -y -z
	AddLine(LineVertex(D3DXVECTOR3(location.x - halfSize, location.y - halfSize, location.z - halfSize), color), LineVertex(D3DXVECTOR3(location.x + halfSize, location.y - halfSize, location.z - halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x + halfSize, location.y - halfSize, location.z - halfSize), color), LineVertex(D3DXVECTOR3(location.x + halfSize, location.y + halfSize, location.z - halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x + halfSize, location.y + halfSize, location.z - halfSize), color), LineVertex(D3DXVECTOR3(location.x - halfSize, location.y + halfSize, location.z - halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x - halfSize, location.y + halfSize, location.z - halfSize), color), LineVertex(D3DXVECTOR3(location.x - halfSize, location.y - halfSize, location.z - halfSize), color));

	// Top
	AddLine(LineVertex(D3DXVECTOR3(location.x - halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x + halfSize, location.y - halfSize, location.z + halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x + halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x + halfSize, location.y + halfSize, location.z + halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x + halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x - halfSize, location.y + halfSize, location.z + halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x - halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x - halfSize, location.y - halfSize, location.z + halfSize), color));

	// Sides
	AddLine(LineVertex(D3DXVECTOR3(location.x - halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x - halfSize, location.y - halfSize, location.z - halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x + halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x + halfSize, location.y - halfSize, location.z - halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x + halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x + halfSize, location.y + halfSize, location.z - halfSize), color));

	AddLine(LineVertex(D3DXVECTOR3(location.x - halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(D3DXVECTOR3(location.x - halfSize, location.y + halfSize, location.z - halfSize), color));

}

/** Adds an AABB-Box to the renderlist */
void BaseLineRenderer::AddAABB(const D3DXVECTOR3& location, const D3DXVECTOR3&  halfSize, const D3DXVECTOR4& color)
{
	AddAABBMinMax(D3DXVECTOR3(	location.x - halfSize.x, 
						location.y - halfSize.y, 
						location.z - halfSize.z), D3DXVECTOR3(	location.x + halfSize.x, 
															location.y + halfSize.y, 
															location.z + halfSize.z), color);
}



void BaseLineRenderer::AddAABBMinMax(const D3DXVECTOR3& min, const D3DXVECTOR3& max, const D3DXVECTOR4& color)
{
	AddLine(LineVertex(D3DXVECTOR3(min.x, min.y, min.z), color), LineVertex(D3DXVECTOR3(max.x, min.y, min.z), color));
	AddLine(LineVertex(D3DXVECTOR3(max.x, min.y, min.z), color), LineVertex(D3DXVECTOR3(max.x, max.y, min.z), color));
	AddLine(LineVertex(D3DXVECTOR3(max.x, max.y, min.z), color), LineVertex(D3DXVECTOR3(min.x, max.y, min.z), color));
	AddLine(LineVertex(D3DXVECTOR3(min.x, max.y, min.z), color), LineVertex(D3DXVECTOR3(min.x, min.y, min.z), color));
													
	AddLine(LineVertex(D3DXVECTOR3(min.x, min.y, max.z), color), LineVertex(D3DXVECTOR3(max.x, min.y, max.z), color));
	AddLine(LineVertex(D3DXVECTOR3(max.x, min.y, max.z), color), LineVertex(D3DXVECTOR3(max.x, max.y, max.z), color));
	AddLine(LineVertex(D3DXVECTOR3(max.x, max.y, max.z), color), LineVertex(D3DXVECTOR3(min.x, max.y, max.z), color));
	AddLine(LineVertex(D3DXVECTOR3(min.x, max.y, max.z), color), LineVertex(D3DXVECTOR3(min.x, min.y, max.z), color));
												
	AddLine(LineVertex(D3DXVECTOR3(min.x, min.y, min.z), color), LineVertex(D3DXVECTOR3(min.x, min.y, max.z), color));
	AddLine(LineVertex(D3DXVECTOR3(max.x, min.y, min.z), color), LineVertex(D3DXVECTOR3(max.x, min.y, max.z), color));
	AddLine(LineVertex(D3DXVECTOR3(max.x, max.y, min.z), color), LineVertex(D3DXVECTOR3(max.x, max.y, max.z), color));
	AddLine(LineVertex(D3DXVECTOR3(min.x, max.y, min.z), color), LineVertex(D3DXVECTOR3(min.x, max.y, max.z), color));
}

/** Adds a ring to the renderlist */
void BaseLineRenderer::AddRingZ(const D3DXVECTOR3& location, float size, const D3DXVECTOR4& color, int res)
{
	std::vector<D3DXVECTOR3> points;
	float step = (float)(D3DX_PI * 2) / (float)res;

	for(int i=0;i<res;i++)
	{
		points.push_back(D3DXVECTOR3(size * sinf(step * i) + location.x, size * cosf(step * i) + location.y, location.z));
	}

	for(unsigned int i=0; i<points.size()-1;i++)
	{
		AddLine(LineVertex(points[i], color), LineVertex(points[i+1], color));
	}

	AddLine(LineVertex(points[points.size()-1], color), LineVertex(points[0], color));
}

/** Draws a wireframe mesh */
void BaseLineRenderer::AddWireframeMesh(const std::vector<ExVertexStruct>& vertices, const std::vector<VERTEX_INDEX>& indices, const D3DXVECTOR4& color, const D3DXMATRIX* world)
{
	for(int i=0;i<indices.size();i+=3)
	{
		D3DXVECTOR3 vx[3];
		for(int v=0;v<3;v++)
		{
			if(world)
				D3DXVec3TransformCoord(&vx[v], vertices[indices[i + v]].Position.toD3DXVECTOR3(), world);
			else
				vx[v] = *vertices[indices[i + v]].Position.toD3DXVECTOR3();
		}

		AddTriangle(vx[0], vx[1], vx[2], color);
	}
}