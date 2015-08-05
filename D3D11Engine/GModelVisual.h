#pragma once
#include "gvisual.h"
#include "WorldConverter.h"

struct NodeAttachmentInfo
{
	NodeAttachmentInfo();
	~NodeAttachmentInfo();

	BaseConstantBuffer* NodeCB;
	VobInstanceInfo data;
};

class GSkeletalMeshVisual;
class GModelVisual :
	public GVisual
{
public:
	GModelVisual(zCVisual* sourceVisual);
	~GModelVisual(void);

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

	/** Called when we are done drawing */
	virtual void OnEndDraw();

protected:
	/** Draws the node attachments */
	virtual void DrawNodeAttachments(const RenderInfo& info, std::vector<D3DXMATRIX>& boneTransforms);
	
	/** Updates the bone-CB */
	void UpdateBoneConstantBuffer();

	/** Prototypes of this visual */
	std::vector<GSkeletalMeshVisual*> SkeletalMeshes;

	/** Map of visuals attached to nodes */
	std::map<int, std::pair<NodeAttachmentInfo*, GVisual *>> NodeAttachments;

	/** Constantbuffer for the bone-matrices of this */
	BaseConstantBuffer* BoneConstantBuffer;
	std::vector<D3DXMATRIX> BoneState;

	/** zCModels only have a single vob attached to them.
		This is valid after the first drawcall to this. */
	GVobObject* OwningVob;

};

