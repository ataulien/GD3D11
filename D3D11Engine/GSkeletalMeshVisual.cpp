#include "pch.h"
#include "GSkeletalMeshVisual.h"
#include "WorldConverter.h"
#include "zCModel.h"
#include "GVobObject.h"
#include "zCVob.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"

GSkeletalMeshVisual::GSkeletalMeshVisual(zCModelMeshLib* sourceProto)
{
	// Extract the info from the visual in the games memory
	WorldConverter::ExtractSkeletalMeshFromProto(sourceProto, &VisualInfo);

	// Init object
	BoneTransforms = NULL;
}


GSkeletalMeshVisual::~GSkeletalMeshVisual(void)
{
}

/** Draws the visual for the given vob */
void GSkeletalMeshVisual::DrawVisual(const RenderInfo& info)
{
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
void GSkeletalMeshVisual::SetBoneTransforms(const std::vector<D3DXMATRIX>* boneTransforms)
{
	BoneTransforms = boneTransforms;
}