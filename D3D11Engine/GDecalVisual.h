#pragma once
#include "gvisual.h"
class GDecalVisual :
	public GVisual
{
public:
	GDecalVisual(zCVisual* sourceVisual);
	~GDecalVisual(void);

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

protected:
	/** Draws a single particle instance */
	virtual void DrawDecalInstance(const RenderInfo& info);

};

