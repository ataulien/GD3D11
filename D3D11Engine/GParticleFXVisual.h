#pragma once
#include "gvisual.h"
#include "WorldConverter.h"

class GParticleFXVisual :
	public GVisual
{
public:
	GParticleFXVisual(zCVisual* sourceVisual);
	~GParticleFXVisual(void);

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

protected:
	/** Draws a single particle instance */
	virtual void DrawParticleInstance(const RenderInfo& info, const ParticleInstanceInfo& instance, const D3DXVECTOR3& positionWorld);
};

