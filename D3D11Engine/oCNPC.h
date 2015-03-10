#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"

class oCNPC : public zCVob
{
public:
	oCNPC(void);
	~oCNPC(void);

	void ResetPos(const D3DXVECTOR3& pos)
	{
		XCALL(GothicMemoryLocations::oCNPC::ResetPos);
	}
};

