#pragma once
#include "gvisual.h"
#include "WorldConverter.h"

class GSkeletalMeshVisual;
class GModelVisual :
	public GVisual
{
public:
	GModelVisual(zCVisual* sourceVisual);
	~GModelVisual(void);

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

protected:
	/** Draws the node attachments */
	virtual void DrawNodeAttachments(const RenderInfo& info, std::vector<D3DXMATRIX>& boneTransforms);
	
	/** Prototypes of this visual */
	std::vector<GSkeletalMeshVisual*> SkeletalMeshes;

	/** Map of visuals attached to nodes */
	std::map<int, GVisual *> NodeAttachments;
};

