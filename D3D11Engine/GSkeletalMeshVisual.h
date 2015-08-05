#pragma once
#include "gvisual.h"
#include "WorldConverter.h"

class GMaterial;
class zCModelPrototype;
class GSkeletalMeshVisual : public GVisual
{
public:
	GSkeletalMeshVisual(zCModelMeshLib* sourceProto);
	~GSkeletalMeshVisual(void);

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

	/** Sets the currently used bone-transforms */
	void SetBoneTransforms(const std::vector<D3DXMATRIX>* boneTransforms, BaseConstantBuffer* boneCB);
protected:
	/** Structure that holds mesh information about this visual */
	SkeletalMeshVisualInfo VisualInfo;

	/** Pointer to the matrices this should use for rendering */
	const std::vector<D3DXMATRIX>* BoneTransforms;
	BaseConstantBuffer* BoneConstantBuffer;

	/** Pipelinestate for this part */
	std::vector<PipelineState*> ImmediatePipelineStates;
};

