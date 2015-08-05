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

	DrawnThisFrame = false;
}


GVisual::~GVisual(void)
{
}

/** Just adds a static instance */
int* GVisual::AddStaticInstance(const VobInstanceRemapInfo& remapInfo)
{
	// Check if this is the first time we are rendering this
	if(!DrawnThisFrame)
	{
		OnBeginDraw();
		DrawnThisFrame = true;
	}

	return NULL;
}

/** Draws the visual for the given vob */
void GVisual::DrawVisual(const RenderInfo& info)
{
	// Check if this is the first time we are rendering this
	if(!DrawnThisFrame)
	{
		OnBeginDraw();
		DrawnThisFrame = true;
	}

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

/** Called when we are done drawing */
void GVisual::OnEndDraw()
{
	DrawnThisFrame = false;
}