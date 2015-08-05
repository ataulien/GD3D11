#include "pch.h"
#include "GModelVisual.h"
#include "GVobObject.h"
#include "zCVob.h"
#include "GSkeletalMeshVisual.h"
#include "zCModel.h"
#include "GGame.h"
#include "GWorld.h"

const int NUM_MAX_BONES = 96;

NodeAttachmentInfo::NodeAttachmentInfo()
{
	Engine::GraphicsEngine->CreateConstantBuffer(&NodeCB, NULL, sizeof(VobInstanceInfo));
}

NodeAttachmentInfo::~NodeAttachmentInfo()
{
	delete NodeCB; NodeCB = NULL;
}

GModelVisual::GModelVisual(zCVisual* sourceVisual) : GVisual(sourceVisual)
{
	zCModel* model = (zCModel *)SourceVisual;
	OwningVob = NULL;

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

	BoneConstantBuffer = NULL;
}


GModelVisual::~GModelVisual(void)
{
	for(auto it=NodeAttachments.begin();it!=NodeAttachments.end();it++)
	{
		delete (*it).second.first;
	}
}

/** Updates the bone-CB */
void GModelVisual::UpdateBoneConstantBuffer()
{
	zCModel* model = (zCModel*)SourceVisual;
	
	// Refresh state cache
	BoneState.clear();
	model->GetBoneTransforms(&BoneState);

	// Init bone-constantbuffer
	if(!BoneConstantBuffer)
		Engine::GraphicsEngine->CreateConstantBuffer(&BoneConstantBuffer, NULL, sizeof(D3DXMATRIX) * BoneState.size());

	// Push the data to the GPU
	BoneConstantBuffer->UpdateBufferDeferred(&BoneState[0]);
}

/** Draws the visual for the given vob */
void GModelVisual::DrawVisual(const RenderInfo& info)
{
	if(!Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes)
		return;

	zCModel* model = (zCModel*)SourceVisual;

	// First-person mode?
	if(model->GetDrawHandVisualsOnly())
		return; // Not supported yet


/*#ifndef DEBUG_DRAW_VISUALS
	return;
#endif*/

	GVisual::DrawVisual(info);

	// Update attachments
	model->UpdateAttachedVobs();
	model->UpdateMeshLibTexAniState();

	if(!OwningVob)
	{
		OwningVob = info.CallingVob;

		// Set fatness to the vob
		// TODO: Check every frame if it changed to catch
		//		 Fatness changing on the fly
		float fatness = model->GetModelFatness();
		OwningVob->SetVobInstanceGPSlot(0, *(DWORD *)&fatness);
		OwningVob->UpdateVobConstantBuffer();
	}
	
	D3DXMATRIX world;
	D3DXMATRIX scale;
	D3DXVECTOR3 modelScale = model->GetModelScale();
	D3DXMatrixScaling(&scale, modelScale.x, modelScale.y, modelScale.z);
	world = *info.WorldMatrix * scale;

	//D3DXMATRIX prevCM = zCCamera::GetCamera()->GetTransform(zCCamera::TT_WORLD);
	//zCCamera::GetCamera()->SetTransform(zCCamera::TT_WORLD, world);

	// Update bone-CB if we have active animations or this is the first time this gets rendered
	if((model->GetNumActiveAnimations() || !BoneConstantBuffer)
		&& !(model->IdleAnimationRunning())) // Don't update if we don't have skeletal parts and are doing an idle-animation // FIXME: Mobs with real idle-animations!
		UpdateBoneConstantBuffer();

	// Loop through meshes and draw them
	for(auto it = SkeletalMeshes.begin();it != SkeletalMeshes.end(); it++)
	{
		(*it)->SetBoneTransforms(&BoneState, BoneConstantBuffer);
		(*it)->DrawVisual(info);
	}

	// Draw attachments
	//DrawNodeAttachments(info, BoneState);

	// Not sure why this is needed, but it messes up other vob drawings if not reset
	//zCCamera::GetCamera()->SetTransform(zCCamera::TT_WORLD, prevCM);
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

		
		GVisual* myNodeVisual = NodeAttachments[i].second;

		// Check if this is loaded or still the same
		if(!myNodeVisual || (NodeAttachments.find(i) == NodeAttachments.end()) || (node->NodeVisual != myNodeVisual->GetSourceVisual()))
		{
			// It's not, extract it
			myNodeVisual = Engine::Game->GetWorld()->GetVisualFrom(node->NodeVisual);
			if(!myNodeVisual)
				myNodeVisual = Engine::Game->GetWorld()->CreateVisualFrom(node->NodeVisual);

			NodeAttachments[i].second = myNodeVisual;

			if(!NodeAttachments[i].first)
				NodeAttachments[i].first = new NodeAttachmentInfo;
		}

		NodeAttachmentInfo* myNodeInfo = NodeAttachments[i].first;

		VobInstanceInfo vi;
		vi.color = 0xFFFFFFFF;
		vi.world = world * boneTransforms[i];

		// Only update on change. Checking that is faster than uploading stuff to the GPU every frame.
		if(memcmp(&myNodeInfo->data, &vi, sizeof(vi)) != 0)
			myNodeInfo->NodeCB->UpdateBufferDeferred(&vi);

		// Draw the visual
		if(myNodeVisual)
		{
			myNodeVisual->DrawVisual(RenderInfo(info.CallingVob, &(world * boneTransforms[i]), myNodeInfo->NodeCB));
		}
	}
}

/** Called when we are done drawing */
void GModelVisual::OnEndDraw()
{
	GVisual::OnEndDraw();
}