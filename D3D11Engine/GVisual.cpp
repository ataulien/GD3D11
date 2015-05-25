#include "pch.h"
#include "GVisual.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "GVobObject.h"

GVisual::GVisual(zCVisual* sourceVisual)
{
	SourceVisual = sourceVisual;
	VisualSize = 0.0f;
}


GVisual::~GVisual(void)
{
}

/** Draws the visual for the given vob */
void GVisual::DrawVisual(const RenderInfo& info)
{
	return;
	Engine::GraphicsEngine->GetLineRenderer()->AddPointLocator(info.CallingVob->GetPosition());
}

/** Returns the size of this visual */
float GVisual::GetVisualSize()
{
	return VisualSize;
}

/** Returns the visual this is based on */
zCVisual* GVisual::GetSourceVisual()
{
	return SourceVisual;
}