#include "pch.h"
#include "GWorldMesh.h"
#include "zCBspTree.h"
#include "zCCamera.h"
#include "zCMaterial.h"
#include "D3D7\MyDirectDrawSurface7.h"

GWorldMesh::GWorldMesh(zCBspTree* bspTree)
{
	std::vector<zCPolygon *> polys;

	LogInfo() << "Extracting world";
	
	// Get polygons from best lod-level (World-LOD is only used in Gothic 1)
	bspTree->GetLOD0Polygons(polys);

	// Convert the mesh into sections
	WorldConverter::ConvertWorldMesh(&polys[0], polys.size(), &WorldSections, Engine::GAPI->GetLoadedWorldInfo(), &WrappedWorldMesh);

	LogInfo() << "Done extracting world!";

	// Create our identitiy transforms CB
	D3DXMATRIX id; D3DXMatrixIdentity(&id);
	VS_ExConstantBuffer_PerInstance cb;
	cb.World = id;
	Engine::GraphicsEngine->CreateConstantBuffer(&TransformsCB, &cb, sizeof(VS_ExConstantBuffer_PerInstance));
	
	for(std::map<int, std::map<int, WorldMeshSectionInfo>>::iterator itx = WorldSections.begin(); itx != WorldSections.end(); itx++)
	{
		for(std::map<int, WorldMeshSectionInfo>::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ity++)
		{
			WorldMeshSectionInfo& section = (*ity).second;

			// We need as much pipeline states as we have textures in the mesh
			for(auto it = section.WorldMeshes.begin();it != section.WorldMeshes.end(); it++)
			{
				// Enter a new state for this submesh-part
				PipelineStates.push_back(Engine::GraphicsEngine->CreatePipelineState());

				// Check for alphatest
				if((*it).first.Material->GetAlphaFunc() > 1 || 
					((*it).first.Material->GetTexture() && (*it).first.Material->GetTexture()->HasAlphaChannel()))
					PipelineStates.back()->BaseState.TranspacenyMode = PipelineState::ETransparencyMode::TM_MASKED;

				Engine::GraphicsEngine->SetupPipelineForStage(STAGE_DRAW_WORLD, PipelineStates.back());

				/*PipelineStates.back()->BaseState.DrawCallType = PipelineState::DCT_DrawIndexed;
				PipelineStates.back()->BaseState.VertexBuffers[0] = (*it).second->MeshVertexBuffer;
				PipelineStates.back()->BaseState.IndexBuffer = (*it).second->MeshIndexBuffer;
				PipelineStates.back()->BaseState.NumIndices = (*it).second->Indices.size();
				PipelineStates.back()->BaseState.NumVertices = (*it).second->Vertices.size();*/
				
				PipelineStates.back()->BaseState.DrawCallType = PipelineState::DCT_DrawIndexed;
				PipelineStates.back()->BaseState.VertexBuffers[0] = WrappedWorldMesh->MeshVertexBuffer;
				PipelineStates.back()->BaseState.IndexBuffer = WrappedWorldMesh->MeshIndexBuffer;
				PipelineStates.back()->BaseState.NumIndices = (*it).second->Indices.size();
				PipelineStates.back()->BaseState.NumVertices = (*it).second->Vertices.size();
				PipelineStates.back()->BaseState.IndexOffset = (*it).second->BaseIndexLocation;
				PipelineStates.back()->BaseState.IndexStride = sizeof(UINT);

				PipelineStates.back()->BaseState.SetCB(1, TransformsCB);

				// Huge safety-check to see if gothic didn't mess this up
				if((*it).first.Material &&
					(*it).first.Material->GetTexture() &&
					(*it).first.Material->GetTexture()->GetSurface() &&
					(*it).first.Material->GetTexture()->GetSurface()->GetEngineTexture())
					PipelineStates.back()->BaseState.TextureIDs[0] = (*it).first.Material->GetTexture()->GetSurface()->GetEngineTexture()->GetID();

				// Enter API-Specific values into the state-object
				Engine::GraphicsEngine->FillPipelineStateObject(PipelineStates.back());
			}

		}
	}
}


GWorldMesh::~GWorldMesh(void)
{
	delete WrappedWorldMesh;
}

/** Returns a reference to the loaded sections */
std::map<int, std::map<int, WorldMeshSectionInfo>>& GWorldMesh::GetWorldSections()
{
	return WorldSections;
}

/** Draws the worldmesh */
void GWorldMesh::DrawMesh()
{
	if(!Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
		return;

	int p=0;
	for(std::map<int, std::map<int, WorldMeshSectionInfo>>::iterator itx = WorldSections.begin(); itx != WorldSections.end(); itx++)
	{
		for(std::map<int, WorldMeshSectionInfo>::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ity++)
		{
			WorldMeshSectionInfo& section = (*ity).second;

			// We need as much pipeline states as we have textures in the mesh
			for(auto it = section.WorldMeshes.begin();it != section.WorldMeshes.end(); it++)
			{
				PipelineState* s = PipelineStates[p]; // Get state for this section-part

				// Make sure the texture is loaded
				if(PipelineStates.back()->BaseState.TextureIDs[0] == 0xFFFF)
				{
					if((*it).first.Material && (*it).first.Material->GetTexture())
						if((*it).first.Material->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
							continue;

					if((*it).first.Material &&
						(*it).first.Material->GetTexture() &&
						(*it).first.Material->GetTexture()->GetSurface() &&
						(*it).first.Material->GetTexture()->GetSurface()->GetEngineTexture())
						PipelineStates[p]->BaseState.TextureIDs[0] = (*it).first.Material->GetTexture()->GetSurface()->GetEngineTexture()->GetID();

									// Check for alphatest
					if((*it).first.Material->GetAlphaFunc() > 1 || 
						((*it).first.Material->GetTexture() && (*it).first.Material->GetTexture()->HasAlphaChannel()))
						PipelineStates[p]->BaseState.TranspacenyMode = PipelineState::ETransparencyMode::TM_MASKED;

					Engine::GraphicsEngine->SetupPipelineForStage(STAGE_DRAW_WORLD, PipelineStates[p]);

					// Need to set up that again, because SetupPipelineForStage overwrites some of it
					PipelineStates[p]->BaseState.DrawCallType = PipelineState::DCT_DrawIndexed;
					PipelineStates[p]->BaseState.VertexBuffers[0] = WrappedWorldMesh->MeshVertexBuffer;
					PipelineStates[p]->BaseState.IndexBuffer = WrappedWorldMesh->MeshIndexBuffer;
					PipelineStates[p]->BaseState.NumIndices = (*it).second->Indices.size();
					PipelineStates[p]->BaseState.NumVertices = (*it).second->Vertices.size();
					PipelineStates[p]->BaseState.IndexOffset = (*it).second->BaseIndexLocation;
					PipelineStates[p]->BaseState.IndexStride = sizeof(UINT);

					PipelineStates[p]->BaseState.SetCB(1, TransformsCB);

					Engine::GraphicsEngine->FillPipelineStateObject(PipelineStates[p]);
				}

				// Push into renderqueue
				Engine::GraphicsEngine->PushPipelineState(s);

				p++;
			}
		}
	}

#ifndef DEBUG_DRAW_VISUALS
	return;
#endif

	std::list<WorldMeshSectionInfo*> renderList;
	CollectVisibleSections(renderList);

	// Debug-draw the world mesh
	for(std::list<WorldMeshSectionInfo*>::iterator it = renderList.begin(); it != renderList.end(); it++)
	{
		for(std::map<MeshKey, WorldMeshInfo*>::iterator itm = (*it)->WorldMeshes.begin(); itm != (*it)->WorldMeshes.end();itm++)
		{
			std::vector<VERTEX_INDEX>& indices = (*itm).second->Indices;
			std::vector<ExVertexStruct>& vertices = (*itm).second->Vertices;

			for(int i=0;i<indices.size();i+=3)
			{
				D3DXVECTOR3 vx[3];
				for(int v=0;v<3;v++)
				{
					vx[v] = *vertices[indices[i + v]].Position.toD3DXVECTOR3();				
				}

				// Don't draw too far
				if(D3DXVec3Length(&(vx[0] - Engine::GAPI->GetCameraPosition())) > 3600.0f)
					continue;

				Engine::GraphicsEngine->GetLineRenderer()->AddTriangle(vx[0], vx[1], vx[2], D3DXVECTOR4(0,0,1,1));
			}
			//Engine::GraphicsEngine->GetLineRenderer()->AddWireframeMesh((*itm).second->Vertices, (*itm).second->Indices, D3DXVECTOR4(0,0,1,1));
		}
	}
}

/** Collects visible sections from the current camera perspective */
void GWorldMesh::CollectVisibleSections(std::list<WorldMeshSectionInfo*>& sections)
{
	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	INT2 camSection = WorldConverter::GetSectionOfPos(camPos);

	sections.push_back(&WorldSections[camSection.x][camSection.y]);
	return;

	// run through every section and check for range and frustum
	int sectionViewDist = Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius;
	
	for(std::map<int, std::map<int, WorldMeshSectionInfo>>::iterator itx = WorldSections.begin(); itx != WorldSections.end(); itx++)
	{
		for(std::map<int, WorldMeshSectionInfo>::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ity++)
		{
			WorldMeshSectionInfo& section = (*ity).second;

			// Simple range-check
			if(abs((*itx).first - camSection.x) < sectionViewDist && 
				abs((*ity).first - camSection.y) < sectionViewDist)
			{
				int flags = 15; // Frustum check, no farplane
				if(zCCamera::GetCamera()->BBox3DInFrustum(section.BoundingBox, flags) == ZTCAM_CLIPTYPE_OUT)
					continue;

				sections.push_back(&section);

				//Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax((*ity).second.BoundingBox.Min, (*ity).second.BoundingBox.Max, D3DXVECTOR4(0,0,1,0.5f));
			}			
		}
	}
}