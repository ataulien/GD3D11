#pragma once
#include "GWorldMesh.h"

class D3D11WorldMesh : public GWorldMesh
{
public:
	D3D11WorldMesh(zCBspTree* bspTree);
	virtual ~D3D11WorldMesh(void);


};

