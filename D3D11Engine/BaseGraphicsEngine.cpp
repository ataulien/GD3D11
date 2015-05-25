#include "pch.h"
#include "BaseGraphicsEngine.h"


BaseGraphicsEngine::BaseGraphicsEngine(void)
{
}


BaseGraphicsEngine::~BaseGraphicsEngine(void)
{
}

UINT64 PipelineState::PipelineSortItem::Build(const BaseState_s& state)
{
	UINT64 v = 0;
	UINT32* vi = (UINT32*)&v;

	vi[1] = AssociatedState->BaseState.TextureIDs[0];
	vi[0] = (UINT32)AssociatedState->BaseState.VertexBuffers[0];
	return v;
}