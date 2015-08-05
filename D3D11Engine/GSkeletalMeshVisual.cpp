#include "pch.h"
#include "GSkeletalMeshVisual.h"
#include "WorldConverter.h"
#include "zCModel.h"
#include "GVobObject.h"
#include "zCVob.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "zCMaterial.h"

GSkeletalMeshVisual::GSkeletalMeshVisual(zCModelMeshLib* sourceProto) : GVisual(NULL)
{
	// Extract the info from the visual in the games memory
	WorldConverter::ExtractSkeletalMeshFromProto(sourceProto, &VisualInfo);

	// Init object
	BoneTransforms = NULL;
	BoneConstantBuffer = NULL;

	// Create our pipeline state
	for(auto it = VisualInfo.SkeletalMeshes.begin();it != VisualInfo.SkeletalMeshes.end(); it++)
	{
		for(int i = 0;i<(*it).second.size();i++)
		{
			// Enter a new state for this submesh-part
			ImmediatePipelineStates.push_back(Engine::GraphicsEngine->CreatePipelineState());

			ImmediatePipelineStates.back()->BaseState.DrawCallType = PipelineState::DCT_DrawIndexed;

			if((*it).first->GetAlphaFunc() > 1)
				ImmediatePipelineStates.back()->BaseState.TranspacenyMode = PipelineState::ETransparencyMode::TM_MASKED;

			Engine::GraphicsEngine->SetupPipelineForStage(STAGE_DRAW_SKELETAL, ImmediatePipelineStates.back());
				
			ImmediatePipelineStates.back()->BaseState.VertexBuffers[0] = (*it).second[i]->MeshVertexBuffer;
			ImmediatePipelineStates.back()->BaseState.IndexBuffer = (*it).second[i]->MeshIndexBuffer;
			ImmediatePipelineStates.back()->BaseState.NumIndices = (*it).second[i]->Indices.size();
			ImmediatePipelineStates.back()->BaseState.NumVertices = (*it).second[i]->Vertices.size();


			// Huge safety-check to see if gothic didn't mess this up
			if((*it).first &&
				(*it).first->GetTexture() &&
				(*it).first->GetTexture()->GetSurface() &&
				(*it).first->GetTexture()->GetSurface()->GetEngineTexture())
				ImmediatePipelineStates.back()->BaseState.TextureIDs[0] = (*it).first->GetTexture()->GetSurface()->GetEngineTexture()->GetID();

			

			// Enter API-Specific values into the state-object
			Engine::GraphicsEngine->FillPipelineStateObject(ImmediatePipelineStates.back());
		}
	}
}


GSkeletalMeshVisual::~GSkeletalMeshVisual(void)
{
	Toolbox::DeleteElements(ImmediatePipelineStates);
}

/** Draws the visual for the given vob */
void GSkeletalMeshVisual::DrawVisual(const RenderInfo& info)
{
	GVisual::DrawVisual(info);

	// Draw the mesh using immediate states
	int n=0;
	for(auto it = VisualInfo.SkeletalMeshes.begin();it != VisualInfo.SkeletalMeshes.end(); it++)
	{
		D3DXMATRIX world; D3DXMatrixTranspose(&world, info.WorldMatrix);

		std::vector<SkeletalMeshInfo*>& meshes = (*it).second;
		for(int i=0;i<meshes.size();i++)
		{
			if(ImmediatePipelineStates[n]->BaseState.TextureIDs[0] == 0xFFFF)
			{
				// Only draw if the texture is loaded
				if((*it).first->GetTexture() && (*it).first->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
				{
					n++;
					continue;
				}

				// Get texture ID if everything is allright
				if((*it).first &&
					(*it).first->GetTexture() &&
					(*it).first->GetTexture()->GetSurface() &&
					(*it).first->GetTexture()->GetSurface()->GetEngineTexture())
				{
					ImmediatePipelineStates[n]->BaseState.TextureIDs[0] = (*it).first->GetTexture()->GetSurface()->GetEngineTexture()->GetID();

					// Get Alphatest
					if((*it).first->GetAlphaFunc() > 1 || (*it).first->GetTexture()->HasAlphaChannel())
						ImmediatePipelineStates[n]->BaseState.TranspacenyMode = PipelineState::ETransparencyMode::TM_MASKED;

					Engine::GraphicsEngine->SetupPipelineForStage(STAGE_DRAW_SKELETAL, ImmediatePipelineStates[n]);
				}
			}

			// Clone the state for this mesh
			PipelineState* transientState = Engine::GraphicsEngine->CreatePipelineState(ImmediatePipelineStates[n]);
			transientState->TransientState = true;

			// Input instanceCB and our bones
			transientState->BaseState.ConstantBuffersVS[1] = info.InstanceCB;
			transientState->BaseState.ConstantBuffersVS[2] = BoneConstantBuffer;

			Engine::GraphicsEngine->FillPipelineStateObject(transientState);

			// Push to renderlist
			Engine::GraphicsEngine->PushPipelineState(transientState);

			n++;
		}
	}

	return;
#ifndef DEBUG_DRAW_VISUALS
	return;
#endif

	std::vector<D3DXMATRIX> trans = *BoneTransforms;
	for(int i=0;i<trans.size();i++)
		D3DXMatrixTranspose(&trans[i], &trans[i]);

	// Debug draw the mesh as line-wireframe
	for(auto it = VisualInfo.SkeletalMeshes.begin();it != VisualInfo.SkeletalMeshes.end(); it++)
	{
		D3DXMATRIX world; D3DXMatrixTranspose(&world, info.WorldMatrix);

		std::vector<SkeletalMeshInfo*>& meshes = (*it).second;
		for(int i=0;i<meshes.size();i++)
		{
			std::vector<VERTEX_INDEX>& indices = meshes[i]->Indices;
			std::vector<ExSkelVertexStruct>& vertices = meshes[i]->Vertices;

			for(int i=0;i<indices.size();i+=3)
			{
				D3DXVECTOR3 vx[3];
				for(int v=0;v<3;v++)
				{
					D3DXVECTOR3 position = D3DXVECTOR3(0,0,0);
					ExSkelVertexStruct& input = vertices[indices[i + v]];
					for(int i=0;i<4;i++)
					{
						D3DXVECTOR3 bp; D3DXVec3TransformCoord(&bp, input.Position[i].toD3DXVECTOR3(), &trans[input.boneIndices[i]]);

						position += input.weights[i] * bp;
					}

					D3DXVec3TransformCoord(&position, &position, &world);

					vx[v] = position;				
				}

				// Don't draw too far
				if(D3DXVec3Length(&(vx[0] - Engine::GAPI->GetCameraPosition())) > 2400)
					continue;

				Engine::GraphicsEngine->GetLineRenderer()->AddTriangle(vx[0], vx[1], vx[2], D3DXVECTOR4(1,0,0,1));
			}

			//Engine::GraphicsEngine->GetLineRenderer()->AddWireframeMesh(meshes[i]->Vertices, meshes[i]->Indices, D3DXVECTOR4(0,1,0,1), &world);
		}
	}
}

/** Sets the currently used bone-transforms */
void GSkeletalMeshVisual::SetBoneTransforms(const std::vector<D3DXMATRIX>* boneTransforms, BaseConstantBuffer* boneCB)
{
	BoneTransforms = boneTransforms;
	BoneConstantBuffer = boneCB;
}