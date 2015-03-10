#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

struct zCMeshData
{
	int				numPoly;
	int				numVert;
    int             numFeat;

	zCVertex		**vertList;
	zCPolygon		**polyList;
    zCVertFeature   **featList;

    zCVertex        *vertArray;
    zCPolygon       *polyArray;
    zCVertFeature   *featArray;
};

class zCMesh
{
public:
	zCPolygon** GetPolygons()
	{
		return *(zCPolygon ***)THISPTR_OFFSET(GothicMemoryLocations::zCMesh::Offset_Polygons);
	}

	int GetNumPolygons()
	{
		return *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCMesh::Offset_NumPolygons);
	}

	/*zCPolygon* GetPolyArray()
	{
		return *(zCPolygon **)THISPTR_OFFSET(GothicMemoryLocations::zCMesh::Offset_PolyArray);
	}

	zCPolygon* SharePoly(int i)
	{
		if(GetPolyArray()) 
			return GetPolyArray() + i;
		else
			return GetPolygons()[i];
	}

	void CreateListsFromArrays()
	{
		XCALL(GothicMemoryLocations::zCMesh::CreateListsFromArrays);
	}

	

	zCMeshData* GetData()
	{
		return (zCMeshData*)THISPTR_OFFSET(GothicMemoryLocations::zCMesh::Offset_NumPolygons);
	}*/
};