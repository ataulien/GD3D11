#include "pch.h"
#include "GWorld.h"
#include "zCBspTree.h"
#include "GVobObject.h"
#include "GothicAPI.h"
#include "GVisual.h"
#include "GWorldMesh.h"
#include "zCVisual.h"
#include "zCProgMeshProto.h"
#include "GStaticMeshVisual.h"
#include "GSkeletalMeshVisual.h"
#include "zCModel.h"
#include "GModelVisual.h"
#include "GMorphMeshVisual.h"
#include "GParticleFXVisual.h"
#include "GDecalVisual.h"
#include "zCTexture.h"
#include "ThreadPool.h"
#include "BasicTimer.h"

const int INSTANCING_BUFFER_SIZE = sizeof(VobInstanceRemapInfo) * 1024;

GWorld::GWorld(void)
{
	WorldMesh = NULL;
	VobInstanceBuffer = NULL;
	VobInstanceRemapBuffer = NULL;
	NumRegisteredVobInstances = 0;
	CurrentVobInstanceSlot = 0;
}


GWorld::~GWorld(void)
{
	delete WorldMesh;
	delete VobInstanceBuffer;
	delete VobInstanceRemapBuffer;

	Toolbox::DeleteElements<GVobObject*>(StaticVobList);
	Toolbox::DeleteElements<GVobObject*>(DynamicVobList);
	Toolbox::DeleteElements<zCVob*, GVobObject*>(VobMap);
	Toolbox::DeleteElements<zCBspBase*, BspNodeInfo*>(BspMap);
}


/** Calculates the level of this node and all subnodes */
int BspNodeInfo::CalculateLevels()
{
	if(OriginalNode->IsLeaf())
		NumLevels = 0;
	else
	{
		int f=0, b=0;

		if(Front)
			f = Front->CalculateLevels()+1;

		if(Back)
			b = Back->CalculateLevels()+1;

		NumLevels = std::max(f,b);
	}

	return NumLevels;		
}


/** Renderproc for the worldmesh */
void GWorld::DrawWorldMeshProc()
{
	GWorld* world = Engine::Game->GetWorld();
	BasicTimer t;

	t.Update();
	world->WorldMesh->DrawMesh();
	t.Update();
	Engine::GAPI->GetRendererState()->RendererInfo.Timing.WorldMeshMS = t.GetDelta();
}

/** Renderproc for the vobs */
void GWorld::DrawVobsProc()
{
	GWorld* world = Engine::Game->GetWorld();
	BasicTimer t;
	t.Update();

	// Draw visible BSP-Nodes
	world->BspDrawnVobs.clear();

	// Traverse BSP-Tree and push states
	world->DrawBspTreeVobs(world->BspMap[Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()], 
		Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()->BBox3D, CLIP_FLAGS_NO_FAR);

	// Reset state on the drawn vobs
	world->ResetBspDrawnVobs();

	// Build instancing buffer
	world->BuildVobInstanceRemapBuffer();
	t.Update();
	Engine::GAPI->GetRendererState()->RendererInfo.Timing.VobsMS = t.GetDelta();
}

/** Renderproc for the vobs */
void GWorld::DrawDynamicVobsProc()
{
	GWorld* world = Engine::Game->GetWorld();
	BasicTimer t;
	t.Update();

	// Draw dynamic vobs
	world->DrawDynamicVobs();

	t.Update();
	Engine::GAPI->GetRendererState()->RendererInfo.Timing.SkeletalMeshesMS = t.GetDelta();
}

/** Called on render */
void GWorld::DrawWorldThreaded()
{
	// Frustum-check-functions from gothic need this
	if(zCCamera::GetCamera())
		zCCamera::GetCamera()->Activate();

	// Tell all visuals that the frame has started
	BeginVisualFrame();
		
	// Push rendertasks
	std::vector<std::future<void>> futures;
	futures.push_back(Engine::RenderingThreadPool->enqueue(GWorld::DrawWorldMeshProc));
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
		futures.push_back(Engine::RenderingThreadPool->enqueue(GWorld::DrawVobsProc));

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawDynamicVOBs)
		futures.push_back(Engine::RenderingThreadPool->enqueue(GWorld::DrawDynamicVobsProc));

	// Wait for all of them to finish
	for each (auto && x in futures)
	{
		x.wait();
	}

	// Tell all visuals that the frame has ended
	EndVisualFrame();

	// Draw sky
	Engine::GAPI->GetLoadedWorldInfo()->MainWorld->GetSkyControllerOutdoor()->RenderSkyPre();

	// Draw main world
	Engine::GraphicsEngine->FlushRenderQueue();

	// Draw sun-effects
	Engine::GAPI->GetLoadedWorldInfo()->MainWorld->GetSkyControllerOutdoor()->RenderSkyPost();
}

/** Called on render */
void GWorld::DrawWorld()
{
	DrawWorldThreaded();
	return;

	// Frustum-check-functions from gothic need this
	if(zCCamera::GetCamera())
		zCCamera::GetCamera()->Activate();

	WorldMesh->DrawMesh();

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{

		// Tell all visuals that the frame has started
		BeginVisualFrame();

		// Draw visible BSP-Nodes
		BspDrawnVobs.clear();

		// Frustum-check-functions from gothic need this
		if(zCCamera::GetCamera())
			zCCamera::GetCamera()->Activate();

		DrawBspTreeVobs(BspMap[Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()], 
			Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()->BBox3D, 63);

		// Reset state on the drawn vobs
		ResetBspDrawnVobs();

		// Build instancing buffer
		BuildVobInstanceRemapBuffer();

		// Draw dynamic vobs
		DrawDynamicVobs();

		// Tell all visuals that the frame has ended
		EndVisualFrame();
	}

	Engine::GAPI->GetLoadedWorldInfo()->MainWorld->GetSkyControllerOutdoor()->RenderSkyPre();

	Engine::GraphicsEngine->FlushRenderQueue();

	Engine::GAPI->GetLoadedWorldInfo()->MainWorld->GetSkyControllerOutdoor()->RenderSkyPost();
}

/** Draws all dynamic vobs */
void GWorld::DrawDynamicVobs()
{
	for(int i=0;i<DynamicVobList.size();i++)
	{
		DynamicVobList[i]->DrawVob();
	}
}

/** Begins the frame on visuals */
void GWorld::BeginVisualFrame()
{
	DrawnVisuals.resize(0);
}

/** Ends the frame on visuals */
void GWorld::EndVisualFrame()
{
	for(auto it = VisualMap.begin();it!= VisualMap.end();it++)
		if((*it).second)(*it).second->OnEndDraw();
}

/** Draws all vobs in the given node */
void GWorld::PrepareBspNodeVobPipelineStates(BspNodeInfo* node)
{
	VobInstanceRemapInfo nullInstance;
	nullInstance.InstanceRemapIndex = -1;

	// Draw normal vobs
	for(auto it = node->Vobs.begin(); it != node->Vobs.end(); it++)
	{
		GVobObject* vob = (*it);	

		if(!(*it)->GetVisual()->IsInstancingCapable())
			continue;

		// Get the indexpointer of this visual
		int* idx = (*it)->GetVisual()->AddStaticInstance(nullInstance);
		if(idx)
		{
			std::pair<int*,std::vector<VobInstanceRemapInfo>>* inst = NULL;
			for(int i=0;i<node->InstanceDataCacheVobs.size();i++)
			{
				if(node->InstanceDataCacheVobs[i].first == vob->GetVisual())
				{
					inst = &node->InstanceDataCacheVobs[i].second;
					break;
				}
			}

			if(!inst)
			{
				node->InstanceDataCacheVobs.resize(node->InstanceDataCacheVobs.size()+1);
				node->InstanceDataCacheVobs.back().first = vob->GetVisual();
				inst = &node->InstanceDataCacheVobs.back().second;
			}
			
			inst->first = idx;
			inst->second.push_back(vob->GetInstanceRemapInfo());		
		}
	}

	// Draw small vobs
	for(auto it = node->SmallVobs.begin(); it != node->SmallVobs.end(); it++)
	{
		GVobObject* vob = (*it);

		if(!(*it)->GetVisual()->IsInstancingCapable())
			continue;

		// Get the indexpointer of this visual
		int* idx = (*it)->GetVisual()->AddStaticInstance(nullInstance);
		if(idx)
		{
			std::pair<int*,std::vector<VobInstanceRemapInfo>>* inst = NULL;
			for(int i=0;i<node->InstanceDataCacheVobs.size();i++)
			{
				if(node->InstanceDataCacheVobs[i].first == vob->GetVisual())
				{
					inst = &node->InstanceDataCacheVobs[i].second;
					break;
				}
			}

			if(!inst)
			{
				node->InstanceDataCacheVobs.resize(node->InstanceDataCacheVobs.size()+1);
				node->InstanceDataCacheVobs.back().first = vob->GetVisual();
				inst = &node->InstanceDataCacheVobs.back().second;
			}
			
			inst->first = idx;
			inst->second.push_back(vob->GetInstanceRemapInfo());			
		}
	}

	// Draw indoor vobs
	for(auto it = node->IndoorVobs.begin(); it != node->IndoorVobs.end(); it++)
	{
		GVobObject* vob = (*it);

		if(!(*it)->GetVisual()->IsInstancingCapable())
			continue;

		// Get the indexpointer of this visual
		int* idx = (*it)->GetVisual()->AddStaticInstance(nullInstance);
		if(idx)
		{
			std::pair<int*,std::vector<VobInstanceRemapInfo>>* inst = NULL;
			for(int i=0;i<node->InstanceDataCacheVobs.size();i++)
			{
				if(node->InstanceDataCacheVobs[i].first == vob->GetVisual())
				{
					inst = &node->InstanceDataCacheVobs[i].second;
					break;
				}
			}

			if(!inst)
			{
				node->InstanceDataCacheVobs.resize(node->InstanceDataCacheVobs.size()+1);
				node->InstanceDataCacheVobs.back().first = vob->GetVisual();
				inst = &node->InstanceDataCacheVobs.back().second;
			}
			
			inst->first = idx;
			inst->second.push_back(vob->GetInstanceRemapInfo());		
		}
	}

	// Draw indoor lights
	/*for(auto it = node->IndoorLights.begin(); it != node->IndoorLights.end(); it++)
	{
		GVobObject* vob = (*it);
		vob->DrawVob();
		drawnVisuals.push_back(vob->GetVisual());
	}

	// Draw normal lights
	for(auto it = node->Lights.begin(); it != node->Lights.end(); it++)
	{
		GVobObject* vob = (*it);
		vob->DrawVob();
		drawnVisuals.push_back(vob->GetVisual());
	}*/

	
}

/** Draws all visible vobs in the tree */
void GWorld::PrepareBspTreePipelineStates(BspNodeInfo* base)
{
	if(!base)
		return;

	// Get prepare vobs for this node
	PrepareBspNodeVobPipelineStates(base);

	// Fix up the IDs so they don't contain any doubles
	for(auto it = base->InstanceDataCacheVobs.begin();it != base->InstanceDataCacheVobs.end(); it++)
		Toolbox::RemoveDoubles((*it).second.second);

	for(auto it = base->InstanceDataCacheSmallVobs.begin();it != base->InstanceDataCacheSmallVobs.end(); it++)
		Toolbox::RemoveDoubles((*it).second.second);

	for(auto it = base->InstanceDataCacheIndoorVobs.begin();it != base->InstanceDataCacheIndoorVobs.end(); it++)
		Toolbox::RemoveDoubles((*it).second.second);

	if(base->OriginalNode->IsLeaf())
	{
		// Just draw everything here
		//PrepareBspNodeVobPipelineStates(base);
	}else
	{
		// We are just a node, continue with the tree
		PrepareBspTreePipelineStates(base->Front);
		PrepareBspTreePipelineStates(base->Back);
	}
}

/** Prepares all visible vobs in the tree */
void GWorld::PrepareBspTreeVobs(BspNodeInfo* base)
{
	if(!base) // Little shortcut for better readability later
		return;

	// Fill instancingbuffer with everything that is loaded now
	std::vector<VobInstanceInfo> instances;
	for(auto it = StaticVobList.begin();it!=StaticVobList.end();it++)
	{
		if(!(*it)->GetVisual()->IsInstancingCapable())
			continue;

		// Set the vobs instance index, so we can remap it to it's actual data later
		(*it)->SetInstanceIndex(instances.size());

		instances.push_back((*it)->GetInstanceInfo());
	}

	// Create and initialize global static instance buffer
	Engine::GraphicsEngine->CreateVertexBuffer(&VobInstanceBuffer);

	// Init it as structured buffer for shader access
	VobInstanceBuffer->Init(&instances[0], 
		instances.size() * sizeof(VobInstanceInfo), 
		BaseVertexBuffer::B_SHADER_RESOURCE, 
		BaseVertexBuffer::U_IMMUTABLE, 
		BaseVertexBuffer::CA_NONE, 
		"GlobalVobInstanceBuffer",
		sizeof(VobInstanceInfo));

	// Continue with the tree, if possible
	//PrepareBspTreeVobs(base->Front);
	//PrepareBspTreeVobs(base->Back);
}


/** Draws all visible vobs in the tree */
void GWorld::DrawBspTreeVobs(BspNodeInfo* base, zTBBox3D boxCell, int clipFlags)
{
	if(!base)
		return;

	/*D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
	zTCam_ClipType nodeClip = ZTCAM_CLIPTYPE_OUT;
	float dist = Toolbox::ComputePointAABBDistance(camPos, base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max);

	while(base && base->OriginalNode)
	{
		//Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max, D3DXVECTOR4(1,1,0,1));

		if (clipFlags>0) 
		{
			float yMaxWorld = Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode() ?
				Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()->BBox3D.Max.y :
				10000.0f; // FIXME: Why does this even happen?

			zTBBox3D nodeBox = base->OriginalNode->BBox3D;

			
			if(dist < vobOutdoorDist)
			{
				// Test this node agains the camera frustum. Store the plane it failed against, because it's likely to fail on that in the next frame again
				if(dist > 0)
				{
					nodeClip = Toolbox::BBox3DInFrustumCached(nodeBox, zCCamera::GetCamera()->GetFrustumPlanes(), zCCamera::GetCamera()->GetFrustumSignBits(), base->FrustumFailCache);
					//nodeClip = zCCamera::GetCamera()->BBox3DInFrustum(nodeBox, clipFlags);
				
					if (nodeClip==ZTCAM_CLIPTYPE_OUT) 
						return; // Nothig to see here. Discard this node and the subtree}
				}else
				{
					// If dist is 0.0f, we are inside the box. No need to do a frustumtest for that
					nodeClip = ZTCAM_CLIPTYPE_CROSSING;
				}
			}else
			{
				// Too far
				return;
			}
		}

		// Is the whole node visible? If so, draw everything inside it
		// Also draw it if this is a crossing leaf, or the node isn't too huge
		// Gothics BSP-Trees can get really stupid and huge sometimes, we don't need all the 
		// precision here, it hurts instancing.
		if(base->OriginalNode->IsLeaf() || base->NumLevels < 6 || nodeClip == ZTCAM_CLIPTYPE_IN)
		{
			//Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max, D3DXVECTOR4(0,1,0,1));

			// Draw non instanceable stuff
			DrawBspNodeVobs(base, dist);

			// Draw instanced stuff
			DrawPreparedPipelineStates(base, dist);
			return;
		}else
		{
			//Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max, D3DXVECTOR4(1,1,0,1));

			zCBspNode* node = (zCBspNode *)base->OriginalNode;

			int	planeAxis = node->PlaneSignbits;

			boxCell.Min.y	= node->BBox3D.Min.y;
			boxCell.Max.y	= node->BBox3D.Min.y;

			zTBBox3D tmpbox = boxCell;
			if (D3DXVec3Dot(&node->Plane.Normal, &camPos) > node->Plane.Distance)
			{ 
				if(node->Front) 
				{
					((float *)&tmpbox.Min)[planeAxis] = node->Plane.Distance;

					// Continue the tree
					DrawBspTreeVobs(base->Front, tmpbox, clipFlags);
				}

				((float *)&boxCell.Max)[planeAxis] = node->Plane.Distance;

				// Try back-node
				base = base->Back;

			} else 
			{
				if (node->Back ) 
				{
					((float *)&tmpbox.Max)[planeAxis] = node->Plane.Distance;
					
					// Continue the tree
					DrawBspTreeVobs(base->Back, tmpbox, clipFlags);
				}

				((float *)&boxCell.Min)[planeAxis] = node->Plane.Distance;

				// Try front-node
				base = base->Front;
			}
		}
	}*/

	if(!base) // Little shortcut for better readability later
		return;

	const float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;

	float dist = Toolbox::ComputePointAABBDistance(Engine::GAPI->GetCameraPosition(), base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max);
	if(dist > Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE
		|| dist > vobOutdoorDist)
		return;

	if(base->OriginalNode->IsLeaf() || base->NumLevels < 5)
	{
		// Just draw everything here
		DrawPreparedPipelineStates(base, dist);

		//if(base->NumLevels < 5)
		//	Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max, D3DXVECTOR4(0,0,1,1));
	}else
	{
		// We are just a node, continue with the tree
		DrawBspTreeVobs(base->Front, boxCell, clipFlags);
		DrawBspTreeVobs(base->Back, boxCell, clipFlags);
	}
}

void GWorld::DrawPreparedPipelineStatesRec(BspNodeInfo* base, zTBBox3D boxCell, int clipFlags)
{
	if(!base)
		return;

	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
	zTCam_ClipType nodeClip = ZTCAM_CLIPTYPE_OUT;

	while(base && base->OriginalNode)
	{
		if (clipFlags>0) 
		{
			float yMaxWorld = Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode() ?
				Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()->BBox3D.Max.y :
				10000.0f; // FIXME: Why does this even happen?

			zTBBox3D nodeBox = base->OriginalNode->BBox3D;
			//float nodeYMax = std::min(yMaxWorld, Engine::GAPI->GetCameraPosition().y);
			//nodeYMax = std::max(nodeYMax, base->OriginalNode->BBox3D.Max.y);
			//nodeBox.Max.y = nodeYMax;

			float dist = Toolbox::ComputePointAABBDistance(camPos, base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max);
			if(dist < vobOutdoorDist)
			{
				nodeClip = zCCamera::GetCamera()->BBox3DInFrustum(nodeBox, clipFlags);
				
				if (nodeClip==ZTCAM_CLIPTYPE_OUT) 
					return; // Nothig to see here. Discard this node and the subtree}
			}else
			{
				// Too far
				return;
			}
		}

		// Is the whole node visible? If so, draw everything inside it
		// Also draw it if this is a crossing leaf, or the node isn't too huge
		// Gothics BSP-Trees can get really stupid and huge sometimes, we don't need all the 
		// precision here, it hurts instancing.
		if(base->OriginalNode->IsLeaf())
		{
			//Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max, D3DXVECTOR4(0,1,0,1));

			DrawPreparedPipelineStates(base, 0.0f);
			return;
		}else
		{
			//Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(base->OriginalNode->BBox3D.Min, base->OriginalNode->BBox3D.Max, D3DXVECTOR4(1,1,0,1));

			zCBspNode* node = (zCBspNode *)base->OriginalNode;

			int	planeAxis = node->PlaneSignbits;

			boxCell.Min.y	= node->BBox3D.Min.y;
			boxCell.Max.y	= node->BBox3D.Min.y;

			zTBBox3D tmpbox = boxCell;
			if (D3DXVec3Dot(&node->Plane.Normal, &camPos) > node->Plane.Distance)
			{ 
				if(node->Front) 
				{
					((float *)&tmpbox.Min)[planeAxis] = node->Plane.Distance;

					// Continue the tree
					DrawPreparedPipelineStatesRec(base->Front, tmpbox, clipFlags);
				}

				((float *)&boxCell.Max)[planeAxis] = node->Plane.Distance;

				// Try back-node
				base = base->Back;

			} else 
			{
				if (node->Back ) 
				{
					((float *)&tmpbox.Max)[planeAxis] = node->Plane.Distance;
					
					// Continue the tree
					DrawPreparedPipelineStatesRec(base->Back, tmpbox, clipFlags);
				}

				((float *)&boxCell.Min)[planeAxis] = node->Plane.Distance;

				// Try front-node
				base = base->Front;
			}
		}
	}
}

/** Draws the prepared pipelinestates of a BSP-Node */
void GWorld::DrawPreparedPipelineStates(BspNodeInfo* node, float nodeDistance)
{
	const float vobIndoorDist = Engine::GAPI->GetRendererState()->RendererSettings.IndoorVobDrawRadius;
	const float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
	const float vobOutdoorSmallDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius;

	// Array of our nodes renderlists
	std::vector<std::pair<GVisual*, std::pair<int*,std::vector<VobInstanceRemapInfo>>>>* lists[] = {&node->InstanceDataCacheVobs,&node->InstanceDataCacheSmallVobs, &node->InstanceDataCacheIndoorVobs};

	// Max renderdistance for the elements in the array above
	float distances[] = {vobOutdoorDist, vobOutdoorSmallDist, vobIndoorDist};

	for(int i=0;i<3;i++)
	{
		// Don't render this part if it's too far away
		if(nodeDistance > distances[i])
			continue;

		auto map = lists[i];

		for(auto it = map->begin();it !=map->end(); it++)
		{
			std::vector<VobInstanceRemapInfo>& instances = (*it).second.second;
			GVisual* visual = (*it).first;
			int* idx = (*it).second.first;

			if(!instances.empty())
			{
				int numLeft = instances.size();

				// Add once so we get an updated instance-index
				if(*idx == -1)
				{
					visual->AddStaticInstance(instances[0]);
					numLeft--;
				}

				// Batch the rest			
				if(numLeft)
				{
					int start = RegisteredVobInstances[*idx].second.size();
					RegisteredVobInstances[*idx].second.insert(RegisteredVobInstances[*idx].second.end(),
						&instances[instances.size() - numLeft], &instances[instances.size()]);
					//RegisteredVobInstances[*idx].second.resize(RegisteredVobInstances[*idx].second.size() + numLeft);
					//memcpy(&RegisteredVobInstances[*idx].second[start], &instances[instances.size() - numLeft], numLeft * sizeof(VobInstanceRemapInfo));
					NumRegisteredVobInstances += numLeft;

					(*it).first->OnAddInstances(numLeft, NULL);
				}
			}
		}
	}
}

/** Draws all vobs in the given node */
void GWorld::DrawBspNodeVobs(BspNodeInfo* node, float nodeDistance)
{
	const float dist = FLT_MAX;
	const float vobIndoorDist = Engine::GAPI->GetRendererState()->RendererSettings.IndoorVobDrawRadius;
	const float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
	const float vobOutdoorSmallDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius;
	const float vobSmallSize = Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize;
	const float visualFXDrawRadius = Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius;

	// Draw normal vobs
	if(nodeDistance < vobOutdoorDist)
	{
		for(auto it = node->NonInstanceableVobs.begin(); it != node->NonInstanceableVobs.end(); it++)
		{
			GVobObject* vob = (*it);

			if(!vob->IsAlreadyCollectedFromTree() /*&& D3DXVec3Length(&(vob->GetSourceVob()->GetPositionWorld() - Engine::GAPI->GetCameraPosition())) < dist*/)
			{
				// Just draw // TODO: Implement culling
				vob->DrawVob();
				vob->SetCollectedFromTreeSearch();
				BspDrawnVobs.push_back(vob);
			}
		}
	}

	// Draw small vobs
	if(nodeDistance < vobOutdoorSmallDist)
	{
		for(auto it = node->NonInstanceableSmallVobs.begin(); it != node->NonInstanceableSmallVobs.end(); it++)
		{
			GVobObject* vob = (*it);

			if(!vob->IsAlreadyCollectedFromTree() /*&& D3DXVec3Length(&(vob->GetSourceVob()->GetPositionWorld() - Engine::GAPI->GetCameraPosition())) < dist*/)
			{
				// Just draw // TODO: Implement culling
				vob->DrawVob();
				vob->SetCollectedFromTreeSearch();
				BspDrawnVobs.push_back(vob);
			}
		}
	}

	// Draw indoor vobs
	if(nodeDistance < vobIndoorDist)
	{
		for(auto it = node->NonInstanceableIndoorVobs.begin(); it != node->NonInstanceableIndoorVobs.end(); it++)
		{
			GVobObject* vob = (*it);

			if(!vob->IsAlreadyCollectedFromTree() /*&& D3DXVec3Length(&(vob->GetSourceVob()->GetPositionWorld() - Engine::GAPI->GetCameraPosition())) < dist*/)
			{
				// Just draw // TODO: Implement culling
				vob->DrawVob();
				vob->SetCollectedFromTreeSearch();
				BspDrawnVobs.push_back(vob);
			}
		}
	}

	// Draw indoor lights
	/*if(nodeDistance < visualFXDrawRadius / 2.0f)
	{
		for(auto it = node->IndoorLights.begin(); it != node->IndoorLights.end(); it++)
		{
			GVobObject* vob = (*it);

			if(!vob->IsAlreadyCollectedFromTree())
			{
				// Just draw // TODO: Implement culling
				vob->DrawVob();
				vob->SetCollectedFromTreeSearch();
				BspDrawnVobs.push_back(vob);
			}
		}
	}

	// Draw normal lights
	if(nodeDistance < visualFXDrawRadius)
	{
		for(auto it = node->Lights.begin(); it != node->Lights.end(); it++)
		{
			GVobObject* vob = (*it);

			if(!vob->IsAlreadyCollectedFromTree())
			{
				// Just draw // TODO: Implement culling
				vob->DrawVob();
				vob->SetCollectedFromTreeSearch();
				BspDrawnVobs.push_back(vob);
			}
		}
	}*/
}

/** Extracts all vobs from the bsp-tree, resets old tree */
void GWorld::BuildBSPTree(zCBspTree* bspTree)
{
	// Reset tree
	BspMap.clear();

	// Recursivly go through the tree
	BuildBspTreeHelper(bspTree->GetRootNode());

	// Prepare rendering
	PrepareBspTreeVobs(BspMap[Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()]);

	PrepareBspTreePipelineStates(BspMap[Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()]);

	// Calculate levels of the tree
	BspMap[Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetRootNode()]->CalculateLevels();
}

/** (Re)loads the world mesh */
void GWorld::ExtractWorldMesh(zCBspTree* bspTree)
{
	// Clear old mesh
	delete WorldMesh;

	// Load new one
	WorldMesh = new GWorldMesh(bspTree);
}

/** Helper function for going through the bsp-tree */
void GWorld::BuildBspTreeHelper(zCBspBase* base)
{
	if(!base)
		return;

	// Check if this is already in
	BspNodeInfo* bvi = BspMap[base];

	// Don't ever do this twice!
	if(bvi)
		return;

	// Put into cache
	bvi = new BspNodeInfo;
	BspMap[base] = bvi;

	bvi->OriginalNode = base;

	if(base->IsLeaf())
	{
		zCBspLeaf* leaf = (zCBspLeaf *)base;
				
		bvi->Front = NULL;
		bvi->Back = NULL;
		
		for(int i=0;i<leaf->LeafVobList.NumInArray;i++)
		{
			// Get the vob info for this one
			if(VobMap.find(leaf->LeafVobList.Array[i]) != VobMap.end())
			{
				GVobObject* v = VobMap[leaf->LeafVobList.Array[i]];

				if(v)
				{				
					float vobSmallSize = Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize;

					if(v->GetSourceVob()->IsIndoorVob())
					{
						// Only add once
						if(std::find(bvi->IndoorVobs.begin(), bvi->IndoorVobs.end(), v) == bvi->IndoorVobs.end())
						{
							v->AddParentBSPNode(bvi);

							bvi->IndoorVobs.push_back(v);	
						}

						// Add to non instancing-list if wanted
						if(!v->GetVisual()->IsInstancingCapable())
							bvi->NonInstanceableIndoorVobs.push_back(v);	

					}
					else if(v->GetVisual()->GetVisualSize() < vobSmallSize)
					{
						// Only add once
						if(std::find(bvi->SmallVobs.begin(), bvi->SmallVobs.end(), v) == bvi->SmallVobs.end())
						{
							v->AddParentBSPNode(bvi);

							bvi->SmallVobs.push_back(v);
						}

						// Add to non instancing-list if wanted
						if(!v->GetVisual()->IsInstancingCapable())
							bvi->NonInstanceableSmallVobs.push_back(v);
					}
					else
					{
						// Only add once
						if(std::find(bvi->Vobs.begin(), bvi->Vobs.end(), v) == bvi->Vobs.end())
						{
							v->AddParentBSPNode(bvi);

							bvi->Vobs.push_back(v);
						}

						// Add to non instancing-list if wanted
						if(!v->GetVisual()->IsInstancingCapable())
							bvi->NonInstanceableVobs.push_back(v);
					}
				}
			}
		}
	}else
	{
		zCBspNode* node = (zCBspNode *)base;

		bvi->OriginalNode = base;

		BuildBspTreeHelper(node->Front);
		BuildBspTreeHelper(node->Back);

		// Get all child vobs
		zCBspBase* chd[2] = {node->Front, node->Back};
		for(int i=0;i<2;i++)
		{
			if(!chd[i])
				continue;

			BspNodeInfo* cn = BspMap[chd[i]];
			bvi->Vobs.insert(bvi->Vobs.end(), cn->Vobs.begin(), cn->Vobs.end());
			bvi->IndoorVobs.insert(bvi->IndoorVobs.end(), cn->IndoorVobs.begin(), cn->IndoorVobs.end());
			bvi->SmallVobs.insert(bvi->SmallVobs.end(), cn->SmallVobs.begin(), cn->SmallVobs.end());
			bvi->IndoorLights.insert(bvi->IndoorLights.end(), cn->IndoorLights.begin(), cn->IndoorLights.end());
			bvi->Lights.insert(bvi->Lights.end(), cn->Lights.begin(), cn->Lights.end());

			bvi->NonInstanceableVobs.insert(bvi->NonInstanceableVobs.end(), cn->NonInstanceableVobs.begin(), cn->NonInstanceableVobs.end());
			bvi->NonInstanceableIndoorVobs.insert(bvi->NonInstanceableIndoorVobs.end(), cn->NonInstanceableIndoorVobs.begin(), cn->NonInstanceableIndoorVobs.end());
			bvi->NonInstanceableSmallVobs.insert(bvi->NonInstanceableSmallVobs.end(), cn->NonInstanceableSmallVobs.begin(), cn->NonInstanceableSmallVobs.end());
		}

		// Save front and back to this
		bvi->Front = BspMap[node->Front];
		bvi->Back = BspMap[node->Back];
	}
}

/** Called when the game wanted to add a vob to the world */
void GWorld::AddVob(zCVob* vob, zCWorld* world, bool forceDynamic)
{
	// Don't need a vob without visual
	if(!vob->GetVisual())
		return;

	GVobObject* v = VobMap[vob];
	GVisual* visual = GetVisualFrom(vob->GetVisual());

	if(v) // Check if we already have this vob
		return; // We do, skip it!

	// Check if this visual was already loaded
	if(!visual)
	{
		// No, load it now.
		visual = CreateVisualFrom(vob->GetVisual());

		// Failed to load the visual?
		if(!visual)
		{
			LogError() << "Failed to load visual: " << vob->GetVisual()->GetObjectName();
			return; 
		}
	}


	
	// If we got here, v must be NULL.
	v = new GVobObject(vob, visual);

	if(!visual->IsInstancingCapable())
		forceDynamic = true; // FIXME: That's not optimal for all vobs! (Mobs like benches for example)

	// Insert into vob-map
	VobMap[vob] = v;
	RegisterSingleVob(v, forceDynamic || !BspMap.empty());
}

/** Creates the right type of visual from the source */
GVisual* GWorld::CreateVisualFrom(zCVisual* sourceVisual)
{
	GVisual* visual;
	switch(sourceVisual->GetVisualType())
	{
	case zCVisual::VT_PROGMESHPROTO:
		visual = new GStaticMeshVisual(sourceVisual);
		break;

	case zCVisual::VT_MODEL:
		visual =  new GModelVisual(sourceVisual);
		break;

	case zCVisual::VT_MORPHMESH:
		visual =  new GMorphMeshVisual(sourceVisual);
		break;

	case zCVisual::VT_PARTICLE_FX:
		visual =  new GParticleFXVisual(sourceVisual);
		break;

	case zCVisual::VT_DECAL:
		visual =  new GDecalVisual(sourceVisual);
		break;

	default:
		visual =  NULL;
	}

	// Register in visualmap straight away
	VisualMap[sourceVisual] = visual;

	return visual;
}

/** Called when a vob moved */
void GWorld::OnVobMoved(zCVob* vob)
{
	// Get GVobObject for this vob
	auto gv = VobMap.find(vob);

	if(gv != VobMap.end() && gv->second)
	{
		// Send data to the GPU
		gv->second->UpdateVobConstantBuffer();
	}
}

/** Returns the matching GVisual from the given zCVisual */
GVisual* GWorld::GetVisualFrom(zCVisual* sourceVisual)
{
	auto it = VisualMap.find(sourceVisual);
	
	if(it != VisualMap.end())
		return (*it).second;

	return NULL;
}

/** Creates a model-proto form the given object */
GSkeletalMeshVisual* GWorld::CreateModelProtoFrom(zCModelMeshLib* sourceProto)
{
	GSkeletalMeshVisual* proto = new GSkeletalMeshVisual(sourceProto);
	ModelProtoMap[sourceProto] = proto;

	return proto;
}

/** Returns the matching GSkeletalMeshVisual from the given zCModelPrototype */
GSkeletalMeshVisual* GWorld::GetModelProtoFrom(zCModelMeshLib* sourceProto)
{
	return ModelProtoMap[sourceProto];
}

/** Registers a single vob in all needed data structures */
void GWorld::RegisterSingleVob(GVobObject* vob, bool forceDynamic)
{
	if(forceDynamic)
	{
		// Put into dynamic list if wanted
		DynamicVobList.push_back(vob);
	}else
	{
		StaticVobList.push_back(vob);
	}

	vob->SetDynamic(forceDynamic);
}

/** Unregisters a single vob from all data structures */
void GWorld::UnregisterSingleVob(GVobObject* vob)
{
	auto dit = std::find(DynamicVobList.begin(), DynamicVobList.end(), vob);
	auto sit = std::find(StaticVobList.begin(), StaticVobList.end(), vob);

	// Remove from our static/dynamic vob-lists
	if(dit != DynamicVobList.end())
		DynamicVobList.erase(dit);

	if(sit != StaticVobList.end())
		StaticVobList.erase(sit);

	// Remove from BSP-Tree
	const std::vector<BspNodeInfo*>& parentNodes = vob->GetParentBSPNodes();
	for(auto it = parentNodes.begin(); it != parentNodes.end(); it++)
	{
		(*it)->RemoveVob(vob);
	}

	// Free memory
	delete vob;
}

/** Removes a vob from the world */
bool GWorld::RemoveVob(zCVob* vob, zCWorld* world)
{
	GVobObject* obj = VobMap[vob];

	if(obj) // Not registered here?
	{
		// Remove from voblist
		UnregisterSingleVob(obj);
	}

	// Remove from map (Still do it if obj is NULL)
	VobMap.erase(vob);

	return obj != NULL;
}

/** Resets all vobs in the BspDrawnVobs-Vector */
void GWorld::ResetBspDrawnVobs()
{
	for(auto it = BspDrawnVobs.begin(); it != BspDrawnVobs.end(); it++)
	{
		(*it)->ResetTreeSearchState();
	}
}

/** Registers a vob instance at the given slot.
		Enter -1 for a new slot.
		Returns the used slot. */
int GWorld::RegisterVobInstance(int slot, const VobInstanceRemapInfo& instance, int* instanceTypeOffset)
{
	if(slot == -1)
	{
		slot = CurrentVobInstanceSlot;

		if(CurrentVobInstanceSlot == RegisteredVobInstances.size())
			RegisteredVobInstances.push_back(std::make_pair(instanceTypeOffset,std::vector<VobInstanceRemapInfo>()));
		else
			RegisteredVobInstances[slot].first = instanceTypeOffset;

		CurrentVobInstanceSlot++;
	}

	RegisteredVobInstances[slot].second.push_back(instance);
	NumRegisteredVobInstances++;

	return slot;
}

/** Builds the vob instancebuffer */
void GWorld::BuildVobInstanceRemapBuffer()
{
	UINT size = NumRegisteredVobInstances * sizeof(VobInstanceRemapInfo);

	if(!VobInstanceRemapBuffer)
	{
		Engine::GraphicsEngine->CreateVertexBuffer(&VobInstanceRemapBuffer);

		// Init it
		VobInstanceRemapBuffer->Init(NULL, StaticVobList.size() * sizeof(VobInstanceRemapInfo), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	// Too small?
	if(VobInstanceRemapBuffer->GetSizeInBytes() < size)
	{
		// Recreate the buffer
		delete VobInstanceRemapBuffer;
		Engine::GraphicsEngine->CreateVertexBuffer(&VobInstanceRemapBuffer);

		// Create a new buffer with the size doubled
		XLE(VobInstanceRemapBuffer->Init(NULL, size * 2, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE));
	}

	// Fill buffer
	byte* mappedData;
	UINT bsize;
	if (XR_SUCCESS == VobInstanceRemapBuffer->MapDeferred(BaseVertexBuffer::EMapFlags::M_WRITE_DISCARD, (void**)&mappedData, &bsize))
	{
		MappedInstanceData = mappedData;
		// Copy data
		int off = 0;
		for(int i=0;i<RegisteredVobInstances.size();i++)
		{
			if(!RegisteredVobInstances[i].second.empty())
			{
				//Toolbox::RemoveDoubles(RegisteredVobInstances[i].second);

				// Get the offset to the visual
				*RegisteredVobInstances[i].first = off;
				memcpy(mappedData + off * sizeof(VobInstanceRemapInfo), &RegisteredVobInstances[i].second[0], RegisteredVobInstances[i].second.size() * sizeof(VobInstanceRemapInfo));

				// Increase offset
				off += RegisteredVobInstances[i].second.size();

				// Clear vector, but keep memory allocated
				RegisteredVobInstances[i].second.resize(0);
			}
		}

		VobInstanceRemapBuffer->UnmapDeferred();
	}

	//RegisteredVobInstances.resize(0);
	NumRegisteredVobInstances = 0;
	CurrentVobInstanceSlot = 0;
}