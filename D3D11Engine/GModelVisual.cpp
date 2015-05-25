#include "pch.h"
#include "GModelVisual.h"
#include "GVobObject.h"
#include "zCVob.h"
#include "GSkeletalMeshVisual.h"
#include "zCModel.h"
#include "GGame.h"
#include "GWorld.h"

GModelVisual::GModelVisual(zCVisual* sourceVisual) : GVisual(sourceVisual)
{
	zCModel* model = (zCModel *)SourceVisual;

	// Extract registered modelprotos
	for(int i=0;i<model->GetMeshLibList()->NumInArray;i++)
	{
		zCModelMeshLib* proto = model->GetMeshLibList()->Array[i]->MeshLibPtr;

		// Get the proto if it was already loaded
		GSkeletalMeshVisual* myProto = Engine::Game->GetWorld()->GetModelProtoFrom(proto);

		if(!myProto) // Load, if needed
			myProto = Engine::Game->GetWorld()->CreateModelProtoFrom(proto);

		SkeletalMeshes.push_back(myProto);
	}
}


GModelVisual::~GModelVisual(void)
{
}

/** Draws the visual for the given vob */
void GModelVisual::DrawVisual(const RenderInfo& info)
{
#ifndef DEBUG_DRAW_VISUALS
	return;
#endif

	std::vector<D3DXMATRIX> boneTransforms;
	zCModel* model = (zCModel *)SourceVisual;

	// Get bone tranforms
	model->GetBoneTransforms(&boneTransforms);

	for(auto it = SkeletalMeshes.begin();it != SkeletalMeshes.end(); it++)
	{
		(*it)->SetBoneTransforms(&boneTransforms);
		(*it)->DrawVisual(info);
	}

	// Draw attachments
	DrawNodeAttachments(info, boneTransforms);
}

/** Draws the node attachments */
void GModelVisual::DrawNodeAttachments(const RenderInfo& info, std::vector<D3DXMATRIX>& boneTransforms)
{
	D3DXMATRIX world; // Get main world matrix
	info.CallingVob->GetSourceVob()->GetWorldMatrix(&world);

	for(unsigned int i=0;i<boneTransforms.size();i++)
	{
		// Check for new visual
		zCModel* visual = (zCModel *)SourceVisual;
		zCModelNodeInst* node = visual->GetNodeList()->Array[i];

		if(!node->NodeVisual)
			continue; // Happens when you pull your sword for example

		GVisual* myNodeVisual = NodeAttachments[i];

		// Check if this is loaded or still the same
		if(!myNodeVisual || (NodeAttachments.find(i) == NodeAttachments.end()) || (node->NodeVisual != NodeAttachments[i]->GetSourceVisual()))
		{
			// It's not, extract it
			myNodeVisual = Engine::Game->GetWorld()->GetVisualFrom(node->NodeVisual);
			if(!myNodeVisual)
				myNodeVisual = Engine::Game->GetWorld()->CreateVisualFrom(node->NodeVisual);

			NodeAttachments[i] = myNodeVisual;
		}

		// Draw the visual
		if(myNodeVisual)
		{
			myNodeVisual->DrawVisual(RenderInfo(info.CallingVob, &(world * boneTransforms[i])));
		}
	}
}