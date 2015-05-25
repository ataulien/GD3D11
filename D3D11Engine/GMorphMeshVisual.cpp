#include "pch.h"
#include "GMorphMeshVisual.h"
#include "zCMorphMesh.h"
#include "zCProgMeshProto.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"

GMorphMeshVisual::GMorphMeshVisual(zCVisual* sourceVisual) : GVisual(sourceVisual)
{

}


GMorphMeshVisual::~GMorphMeshVisual(void)
{

}

/** Draws the visual for the given vob */
void GMorphMeshVisual::DrawVisual(const RenderInfo& info)
{
#ifndef DEBUG_DRAW_VISUALS
	return;
#endif

	zCMorphMesh* morphMesh = (zCMorphMesh*)SourceVisual;
	zCProgMeshProto* msh = morphMesh->GetMorphMesh();

	// Debug draw morph mesh

	// TODO: Only do this if the animation changed the mesh!
	morphMesh->AdvanceAnis();
	morphMesh->CalcVertexPositions();

	D3DXVECTOR3* posList = (D3DXVECTOR3 *)msh->GetPositionList()->Array;

	// Get world matrix
	D3DXMATRIX world; D3DXMatrixTranspose(&world, info.WorldMatrix);

	// Construct unindexed mesh
	for(int i=0; i < msh->GetNumSubmeshes(); i++)
	{
		std::vector<D3DXVECTOR3> vertices;

		zCSubMesh& sub = msh->GetSubmeshes()[i];
		vertices.reserve(sub.TriList.NumInArray * 3);

		// Get vertices
		for(int t=0;t<sub.TriList.NumInArray;t++)
		{				
			for(int v=0;v<3;v++)
			{
				/*ExVertexStruct vx;
				vx.Position = posList[sub.WedgeList.Array[msh->GetSubmeshes()[i].TriList.Array[t].wedge[v]].position];
				vx.TexCoord	= msh->GetSubmeshes()[i].WedgeList.Array[sub.TriList.Array[t].wedge[v]].texUV;
				vx.Color = 0xFFFFFFFF;
				vx.Normal = msh->GetSubmeshes()[i].WedgeList.Array[sub.TriList.Array[t].wedge[v]].normal;

				// Do this on GPU probably?
				*vx.Position.toD3DXVECTOR3() += (*vx.Normal.toD3DXVECTOR3()); // TODO: Fatness!
				*/
				D3DXVECTOR3 position = posList[sub.WedgeList.Array[msh->GetSubmeshes()[i].TriList.Array[t].wedge[v]].position];

				// Transform to worldspace
				D3DXVec3TransformCoord(&position, &position, &world);

				vertices.push_back(position);
			}	
		}

		for(int j=0;j<vertices.size();j+=3)
		{
			Engine::GraphicsEngine->GetLineRenderer()->AddTriangle(vertices[j], vertices[j+1], vertices[j+2], D3DXVECTOR4(1,1,0,1)); 
		}
	}
}