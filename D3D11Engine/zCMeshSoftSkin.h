#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCArray.h"
#include "zCProgMeshProto.h"

#pragma pack (push, 1)
struct zTWeightEntry {
	float Weight;
	D3DXVECTOR3	VertexPosition;
	unsigned char NodeIndex;
};
#pragma pack (pop)

class zCOBBox3D;
class zCMeshSoftSkin : public zCProgMeshProto
{
public:

	/*zCArray<int>* GetNodeIndexList()
	{
		return (zCArray<int> *)THISPTR_OFFSET(GothicMemoryLocations::zCMeshSoftSkin::Offset_NodeIndexList);
	}*/

	struct zTNodeWedgeNormal {
		D3DXVECTOR3		Normal;
		int				NodeIndex;
	};

	char* GetVertWeightStream()
	{
		return *(char **)THISPTR_OFFSET(GothicMemoryLocations::zCMeshSoftSkin::Offset_VertWeightStream);
	};
};