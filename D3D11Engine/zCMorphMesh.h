#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCModelTexAniState.h"


class zCMorphMesh
{
public:
	zCProgMeshProto* GetMorphMesh()
	{
		return *(zCProgMeshProto **)THISPTR_OFFSET(GothicMemoryLocations::zCMorphMesh::Offset_MorphMesh);
	}

	zCModelTexAniState* GetTexAniState()
	{
		return (zCModelTexAniState *)THISPTR_OFFSET(GothicMemoryLocations::zCMorphMesh::Offset_TexAniState);
	}

	void CalcVertexPositions()
	{
#ifndef BUILD_GOTHIC_1_08k
		XCALL(GothicMemoryLocations::zCMorphMesh::CalcVertexPositions);
#endif
	}

	void AdvanceAnis()
	{
#ifndef BUILD_GOTHIC_1_08k
		XCALL(GothicMemoryLocations::zCMorphMesh::AdvanceAnis);
#endif
	}

	void UpdateBuffer(BaseVertexBuffer* buffer)
	{

	}
};