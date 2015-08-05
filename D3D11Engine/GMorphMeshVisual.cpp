#include "pch.h"
#include "GMorphMeshVisual.h"
#include "zCMorphMesh.h"
#include "zCProgMeshProto.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "zCMaterial.h"

GMorphMeshVisual::GMorphMeshVisual(zCVisual* sourceVisual) : GVisual(sourceVisual)
{
	CacheIndex = 0;

	zCMorphMesh* morphMesh = (zCMorphMesh*)SourceVisual;
	zCProgMeshProto* msh = morphMesh->GetMorphMesh();

	// Make space in the vertexbuffercache for all submeshes
	VertexBufferCache.resize(msh->GetNumSubmeshes());

	// Create our pipeline state
	for(int m=0;m<msh->GetNumSubmeshes();m++)
	{
		// Enter a new state for this submesh-part
		ImmediatePipelineStates.push_back(Engine::GraphicsEngine->CreatePipelineState());

		ImmediatePipelineStates.back()->BaseState.DrawCallType = PipelineState::DCT_DrawTriangleList;

		zCMaterial* mat = msh->GetSubmeshes()[m].Material;
		if(mat->GetAlphaFunc() > 1)
			ImmediatePipelineStates.back()->BaseState.TranspacenyMode = PipelineState::ETransparencyMode::TM_MASKED;

		Engine::GraphicsEngine->SetupPipelineForStage(STAGE_DRAW_WORLD, ImmediatePipelineStates.back());

		ImmediatePipelineStates.back()->BaseState.VertexBuffers[0] = NULL; // Filled later
		ImmediatePipelineStates.back()->BaseState.IndexBuffer = NULL;
		ImmediatePipelineStates.back()->BaseState.NumIndices = 0;
		ImmediatePipelineStates.back()->BaseState.NumVertices = 0;


		// Huge safety-check to see if gothic didn't mess this up
		if(	mat->GetTexture() &&
			mat->GetTexture()->GetSurface() &&
			mat->GetTexture()->GetSurface()->GetEngineTexture())
			ImmediatePipelineStates.back()->BaseState.TextureIDs[0] = mat->GetTexture()->GetSurface()->GetEngineTexture()->GetID();

		// Enter API-Specific values into the state-object
		Engine::GraphicsEngine->FillPipelineStateObject(ImmediatePipelineStates.back());
	}

	int cacheSlot = RequestCacheVB();

	// Pull data out of the morphmesh
	UpdateVertexbuffers(cacheSlot, 0.0f); // TODO: Fatness!
}


GMorphMeshVisual::~GMorphMeshVisual(void)
{
	for(auto it=VertexBufferCache.begin();it!=VertexBufferCache.end();it++)
		Toolbox::DeleteElements((*it));
}

/** Draws the visual for the given vob */
void GMorphMeshVisual::DrawVisual(const RenderInfo& info)
{
	GVisual::DrawVisual(info);

	zCMorphMesh* morphMesh = (zCMorphMesh*)SourceVisual;
	zCProgMeshProto* msh = morphMesh->GetMorphMesh();

	// Make us a new cache-slot, 
	//int cacheSlot = RequestCacheVB();
	int cacheSlot = 0;

	// Pull data out of the morphmesh
	//UpdateVertexbuffers(cacheSlot, 0.0f); // TODO: Fatness!

	// Draw the mesh using immediate states
	int n=0;
	for(int m=0;m<msh->GetNumSubmeshes();m++)
	{
		zCMaterial* mat = msh->GetSubmeshes()[m].Material;

		//if(ImmediatePipelineStates[n]->BaseState.TextureIDs[0] == 0xFFFF)
		{
			// Only draw if the texture is loaded
			if(mat->GetTexture() && mat->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
			{
				n++;
				continue;
			}

			// Get texture ID if everything is allright
			if( mat &&
				mat->GetTexture() &&
				mat->GetTexture()->GetSurface() &&
				mat->GetTexture()->GetSurface()->GetEngineTexture())
				ImmediatePipelineStates[n]->BaseState.TextureIDs[0] = mat->GetTexture()->GetSurface()->GetEngineTexture()->GetID();
		}

		// Clone the state for this mesh
		PipelineState* transientState = Engine::GraphicsEngine->CreatePipelineState(ImmediatePipelineStates[n]);
		transientState->TransientState = true;

		// Place the vertexbuffer from our cacheslot
		transientState->BaseState.VertexBuffers[0] = VertexBufferCache[m][cacheSlot];

		// Compute amount of vertices in that submesh
		int numVertices = msh->GetSubmeshes()[m].TriList.NumInArray * 3;
		transientState->BaseState.NumVertices = numVertices;

		// Input instanceCB and our bones
		transientState->BaseState.ConstantBuffersVS[1] = info.InstanceCB;

		// Update state
		Engine::GraphicsEngine->FillPipelineStateObject(transientState);

		// Push to renderlist
		Engine::GraphicsEngine->PushPipelineState(transientState);

		n++;
	}

#ifndef DEBUG_DRAW_VISUALS
	return;
#endif

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

/** Updates vertexbuffers in the cache at the given slot */
void GMorphMeshVisual::UpdateVertexbuffers(int slot, float fatness)
{
	zCMorphMesh* morphMesh = (zCMorphMesh*)SourceVisual;
	zCProgMeshProto* msh = morphMesh->GetMorphMesh();

	for(int i=0; i < msh->GetNumSubmeshes(); i++)
	{
		D3DXVECTOR3* posList = (D3DXVECTOR3 *)msh->GetPositionList()->Array;
		zCSubMesh& sub = msh->GetSubmeshes()[i];

		// Static so we save memory allocation time
		static std::vector<ExVertexStruct> vertices;
		vertices.resize(sub.TriList.NumInArray * 3);

		// Get vertices
		int vxn=0;
		for(int t=0;t<sub.TriList.NumInArray;t++)
		{			
			// Reversed direction, so we fit with our winding-order
			for(int v=2;v>=0;v--)
			{
				ExVertexStruct vx;
				vx.Position = posList[sub.WedgeList.Array[msh->GetSubmeshes()[i].TriList.Array[t].wedge[v]].position];
				vx.TexCoord	= msh->GetSubmeshes()[i].WedgeList.Array[sub.TriList.Array[t].wedge[v]].texUV;
				vx.Color = 0xFFFFFFFF;
				vx.Normal = msh->GetSubmeshes()[i].WedgeList.Array[sub.TriList.Array[t].wedge[v]].normal;

				// Do this on GPU probably?
				*vx.Position.toD3DXVECTOR3() += (*vx.Normal.toD3DXVECTOR3()) * fatness;

				vertices[vxn] = vx;
				vxn++;
			}	
		}

		// Update vertexbuffer in cache for this submesh
		VertexBufferCache[i][slot]->UpdateBuffer(&vertices[0]);
	}
}

/** Called on a new frame */
void GMorphMeshVisual::OnBeginDraw()
{
	GVisual::OnBeginDraw();


	zCMorphMesh* morphMesh = (zCMorphMesh*)SourceVisual;

	// Update morphmesh. We can do that in OnBeginDraw, since that will only be
	// called if the visual actually is drawn
	morphMesh->AdvanceAnis();
	morphMesh->CalcVertexPositions();

	// Update animated textures
	morphMesh->GetTexAniState()->UpdateTexList();

	// Reset the Cache-Index here, so we can reuse old vertexbuffers
	CacheIndex = 0;
}

/** Called when we are done drawing */
void GMorphMeshVisual::OnEndDraw()
{
	GVisual::OnEndDraw();
}

/** Returns a fresh usable cache-slot. Trys to reuse old vertexbuffers, but will create a new one
if needed */
int GMorphMeshVisual::RequestCacheVB()
{
	// Only check for 0 here, because all have to be the same size anyways
	if(VertexBufferCache[0].size() <= CacheIndex)
	{
		for(int i=0;i<VertexBufferCache.size();i++)
		{
			// Not enough space in VB-Cache, create another buffer
			BaseVertexBuffer* vb; Engine::GraphicsEngine->CreateVertexBuffer(&vb);

			zCMorphMesh* mm = (zCMorphMesh *)SourceVisual;

			// Compute amount of vertices in that submesh
			int numVertices = mm->GetMorphMesh()->GetSubmeshes()[i].TriList.NumInArray * 3;

			// Create buffer and push it to the cache
			vb->Init(NULL, sizeof(ExVertexStruct) * numVertices, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
			VertexBufferCache[i].push_back(vb);
		}
	}

	// Increase cache index for the next call
	CacheIndex++;

	// Return current cache index for this call
	return CacheIndex-1;
}
