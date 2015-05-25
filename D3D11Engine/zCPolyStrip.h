#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCObject.h"

class zCPolyStrip : public zCObject
{
public:
#ifndef BUILD_GOTHIC_1_08k
	zCPolygon* GetPolyList()
	{
		return *(zCPolygon**)THISPTR_OFFSET(GothicMemoryLocations::zCPolyStrip::Offset_PolyList);
	}

	int GetNumPolys()
	{
		return *(int*)THISPTR_OFFSET(GothicMemoryLocations::zCPolyStrip::Offset_NumPolys);
	}


	/** Generates a vertex-buffer from the poly-list */
	void GenerateVertexBuffer(std::vector<ExVertexStruct>& vx)
	{
		int num = GetNumPolys();
		zCPolygon* polyArray = GetPolyList();

		for(int i=0;i<num;i++)
		{
			zCPolygon* poly = &polyArray[i];

			std::vector<ExVertexStruct> polyFan;
			for(int v=0;v<poly->GetNumPolyVertices();v++)
			{
				ExVertexStruct t;
				t.Position = poly->getVertices()[v]->Position;
				t.TexCoord = poly->getFeatures()[v]->texCoord;
				t.Normal = poly->getFeatures()[v]->normal;
				t.Color = poly->getFeatures()[v]->lightStatic;
			}

			// Make a triangle list
			if(!polyFan.empty())
				WorldConverter::TriangleFanToList(&polyFan[0], polyFan.size(), &vx);
		}
	}
#endif
};