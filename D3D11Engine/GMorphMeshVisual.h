#pragma once
#include "GVisual.h"

class GMorphMeshVisual : public GVisual
{
public:
	GMorphMeshVisual(zCVisual* sourceVisual);
	~GMorphMeshVisual(void);

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);
};

