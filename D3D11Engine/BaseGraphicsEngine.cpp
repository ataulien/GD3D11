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
	UINT16* vi = (UINT16*)&v;

	vi[3] = AssociatedState->BaseState.TextureIDs[0];
	vi[2] = (UINT16)AssociatedState->BaseState.VertexBuffers[0];
	vi[1] = AssociatedState->BaseState.PShaderID;
	vi[0] = AssociatedState->BaseState.VShaderID;
	return v;
}