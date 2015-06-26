#include "pch.h"
#include "D3D11GraphicsEngineTest.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "RenderToTextureBuffer.h"
#include "zCTexture.h"
#include <algorithm>
#include "D3D11Texture.h"
#include "zCMaterial.h"
#include "D3D11PfxRenderer.h"
#include "D3D11PShader.h"
#include "AlignedAllocator.h"
#include "GSky.h"
#include "D3D11VertexBuffer.h"
#include "D3D11ConstantBuffer.h"
#include "BaseLineRenderer.h"
#include "D3D11VShader.h"
#include "D3D11ShaderManager.h"

D3D11GraphicsEngineTest::D3D11GraphicsEngineTest(void)
{
}


D3D11GraphicsEngineTest::~D3D11GraphicsEngineTest(void)
{
}


/** Called when we started to render the world */
XRESULT D3D11GraphicsEngineTest::OnStartWorldRendering()
{
	if(Engine::GAPI->GetRendererState()->RendererSettings.DisableRendering)
		return XR_SUCCESS;

	SetDefaultStates();

	PS_LPP = ShaderManager->GetPShader("PS_LPPNormalmappedAlphaTest");

	Engine::GAPI->SetFarPlane(80000.0f);
	Clear(float4(Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor, 0.0f));

	ID3D11RenderTargetView* rtvs[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
	Context->OMSetRenderTargets(2, rtvs, DepthStencilBuffer->GetDepthStencilView());

	
	//if(Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
	//	DrawWorldMeshTest();
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
		DrawSceneLightPrePass();

	SetDefaultStates();

	PfxRenderer->CopyTextureToRTV(GBuffer0_Diffuse->GetShaderResView(), HDRBackBuffer->GetRenderTargetView());
	PfxRenderer->CopyTextureToRTV(GBuffer1_Normals_SpecIntens_SpecPower->GetShaderResView(), HDRBackBuffer->GetRenderTargetView(), INT2(256,256));

	// Set viewport for gothics rendering
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 0.0f;
	vp.Width = (float)GetBackbufferResolution().x;
	vp.Height = (float)GetBackbufferResolution().y;

	Context->RSSetViewports(1, &vp);

	

	return XR_SUCCESS;
}

/** Collects the renderlist for the world-mesh */
void D3D11GraphicsEngineTest::GetWorldMeshRenderList(std::list<std::pair<MeshKey, MeshInfo *>>& list)
{
	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view); // Make sure frustum-culling is working

	// Querry the visible sections from GAPI
	std::list<WorldMeshSectionInfo*> renderList;
	Engine::GAPI->CollectVisibleSections(renderList);

	// Collect meshes from sections
	for(std::list<WorldMeshSectionInfo*>::iterator it = renderList.begin(); it != renderList.end(); it++)
	{
		for(std::map<MeshKey, WorldMeshInfo*>::iterator itm = (*it)->WorldMeshes.begin(); itm != (*it)->WorldMeshes.end();itm++)
		{
			if(!(*itm).first.Texture)
				continue;

			if(!(*itm).first.Texture->GetSurface() || !(*itm).first.Texture->GetSurface()->GetEngineTexture())
			{
				(*itm).first.Texture->CacheIn(0.6f);
				continue;
			}

			list.push_back(std::make_pair((*itm).first, (*itm).second));
		}
	}

	struct cmpstruct
	{
		static bool cmp(const std::pair<MeshKey, MeshInfo *>& a, const std::pair<MeshKey, MeshInfo *>& b)
		{
			if(a.first.Texture->HasAlphaChannel())
				return false; // Render alpha last

			return a.first.Texture < b.first.Texture;
		}
	};

	// Sort by texture
	list.sort(cmpstruct::cmp);
}

void D3D11GraphicsEngineTest::DrawWorldMeshTest()
{

}


/** Draws a world mesh render list for the given stage */
void D3D11GraphicsEngineTest::DrawWorldMeshRenderList(int stage, const std::list<std::pair<MeshKey, MeshInfo *>>& list)
{
	zCTexture* bound = NULL;
	for(std::list<std::pair<MeshKey, MeshInfo *>>::const_iterator it = list.begin(); it != list.end(); it++)
	{
		if((*it).first.Texture != bound)
		{
			MyDirectDrawSurface7* surface = (*it).first.Texture->GetSurface();
			ID3D11ShaderResourceView* srv[2];
			
			// Get diffuse and normalmap
			srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
			srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
			
			// Bind both
			Context->PSSetShaderResources(0,2, srv);
		}

		// Draw the section-part
		DrawVertexBufferIndexedUINT(NULL, NULL, 
			(*it).second->Indices.size(),
			(*it).second->BaseIndexLocation);
	}
}

/** Draws the scene using the light-pre-pass technique */
void D3D11GraphicsEngineTest::DrawSceneLightPrePass()
{
	// Make sure nothing from the previous stage remained
	SetDefaultStates();

	// Collect visible vobs and lights
	static bool s_done = false;
	if(!s_done)
	{
		if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs || 
			Engine::GAPI->GetRendererState()->RendererSettings.EnableDynamicLighting)
		{
			/*if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum ||
				(Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum && FrameVisibleVobs.empty()))
				Engine::GAPI->CollectVisibleVobs(FrameVisibleVobs, FrameVisibleLights);*/

			// Push the data to the GPU
			FillInstancingBuffer(FrameVisibleVobs);

			// Reset the collected vobs for next frame
			MarkVobNonVisibleInFrame(FrameVisibleVobs);

			s_done = true;
		}
	}

	// Collect mesh list for the world
	std::list<std::pair<MeshKey, MeshInfo *>> worldMeshList;
	GetWorldMeshRenderList(worldMeshList);

	/** Init graphics */

	// Vobs need this
	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = true;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	// Set current view-matrix
	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	// Wireframe?
	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}

	// Set shaders
	SetActivePixelShader("PS_LPPNormalmappedAlphaTest");
	SetActiveVertexShader("VS_ExInstancedObj");

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Get reference to visual-map
	const std::hash_map<zCProgMeshProto*, MeshVisualInfo*>& vis = Engine::GAPI->GetStaticMeshVisuals();

	

	/** z-pre-pass */

	// Unbind Pixelshader
	PS_LPP->Apply();

	// Bind GBuffer 2
	Context->OMSetRenderTargets(1, GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	// Set stage
	SetRenderingStage(DES_Z_PRE_PASS);

	// Draw the instances of every visual, if available
	for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
	{
		// Draw all submeshes of this in the world in one batch each
		if(!(*it).second->Instances.empty())
			DrawVisualInstances((*it).second);
	}


	

	SetActiveVertexShader("VS_Ex");
	Engine::GAPI->ResetWorldTransform(); // WorldMesh is always at 0,0,0

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Draw world mesh
	//DrawWorldMeshRenderList(0, worldMeshList);

	/** Diffuse pass */

	// Bind Rendertaget
	Context->OMSetRenderTargets(1, GBuffer0_Diffuse->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	// Set stage
	SetRenderingStage(DES_MAIN);

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_ExInstancedObj");

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Set depth-func to equal only to make use of the z-pre pass
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_EQUAL;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	UpdateRenderStates();

	

	// Draw the instances of every visual, if available
	for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
	{
		if(!(*it).second->Instances.empty())
		{
			// Draw all submeshes of this in the world in one batch each
			DrawVisualInstances((*it).second);

			// Start new frame on the visual, since we don't need the instancing data anymore
			//(*it).second->StartNewFrame();
		}
	}

	

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->DSSetShader(NULL, NULL, NULL);
	Context->HSSetShader(NULL, NULL, NULL);

	SetActiveVertexShader("VS_Ex");
	Engine::GAPI->ResetWorldTransform(); // WorldMesh is always at 0,0,0

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Draw world mesh
	//DrawWorldMeshRenderList(0, worldMeshList);

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	FrameVisibleVobs.clear();
	FrameVisibleLights.clear();
}

/** Draws a StaticMeshVisual instanced */
void D3D11GraphicsEngineTest::DrawVisualInstances(MeshVisualInfo* visual)
{
	// Bind distance information to make fade-out working
	BindDistanceInformationFor(visual);

	// Check if cache exists
	// There should be no visuals without meshes here, so we don't have to check the map for being empty, too.
	if(visual->MeshesCached.empty())
	{
		int i=0; // Counter for materials
		visual->MeshesCached.resize(visual->MeshesByTexture.size());
		for(std::map<MeshKey, std::vector<MeshInfo*>>::iterator it = visual->MeshesByTexture.begin(); it != visual->MeshesByTexture.end(); it++)
		{
			// Add key and vector
			visual->MeshesCached[i].first = (*it).first;
			visual->MeshesCached[i].second = (*it).second;
			i++;
		}
	}

	// Loop through the materials
	for(int m=0;m<visual->MeshesCached.size();m++)
	{
		const MeshKey& key = visual->MeshesCached[m].first;
		std::vector<MeshInfo*>& mlist = visual->MeshesCached[m].second;

		// Bind shader and texture for every submesh (Should only be one, maybe more for very high-poly meshes)
		BindShaderForKey(key);

		// Loop through the submeshes and draw them all in one batch
		for(int i=0;i<mlist.size();i++)
		{
			// Bind tesselation info if possible
			//TryBindPNAENShaderForVisual(visual, mlist[i]);

			// Draw using normal- or PNAEN-Indices whether we have tesselation enabled or not
			if(!ActiveHDS)
			{
				// Draw batch
				DrawInstanced(	mlist[i]->MeshVertexBuffer, 
								mlist[i]->MeshIndexBuffer, 
								mlist[i]->Indices.size(), 
								DynamicInstancingBuffer, 
								sizeof(VobInstanceInfo), 
								visual->Instances.size(), 
								sizeof(ExVertexStruct), 
								visual->StartInstanceNum);
			}else
			{
				// Draw batch tesselated
				DrawInstanced(	mlist[i]->MeshVertexBuffer, 
								mlist[i]->MeshIndexBufferPNAEN, 
								mlist[i]->IndicesPNAEN.size(), 
								DynamicInstancingBuffer, 
								sizeof(VobInstanceInfo), 
								visual->Instances.size(), 
								sizeof(ExVertexStruct), 
								visual->StartInstanceNum);
			}
		}
	}
}

/** Binds the texture for the given mesh-key.
	- Returns false if not cached in */
bool D3D11GraphicsEngineTest::BindShaderForKey(const MeshKey& key)
{
	zCTexture* tx = key.Texture;

	if(!tx)
		return false; // FIXME: Gregs hat has this! Returning here causes it to not render at all

	// Bind texture
	if(tx->CacheIn(0.6f) == zRES_CACHED_IN) // FIXME: Don't always use a texture in a z-pre-pass!
	{
		MyDirectDrawSurface7* surface = tx->GetSurface();
		ID3D11ShaderResourceView* srv[2];

		// Get diffuse and normalmap
		srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
		srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;

		// Bind both
		Context->PSSetShaderResources(0,2, srv);

		if(RenderingStage == DES_Z_PRE_PASS)
		{
			// Force alphatest on vobs for now
			BindShaderForTexturePrePass(tx, true);
		}else
		{
			BindShaderForTextureDiffusePass(tx);
		}

		// Get and update info if neccessary
		MaterialInfo* info = key.Info;
		if(!info->Constantbuffer)
			info->UpdateConstantbuffer();

		// Bind info to pixel shader
		info->Constantbuffer->BindToPixelShader(2);
	}
}

/** Binds the right shader for the given texture */
void D3D11GraphicsEngineTest::BindShaderForTexturePrePass(zCTexture* texture, bool forceAlphaTest, int zMatAlphaFunc)
{
	D3D11PShader* active = ActivePS;
	D3D11PShader* newShader = ActivePS;

	if(texture->HasAlphaChannel() || forceAlphaTest)
	{
		newShader = PS_DiffuseAlphatest;	
	}else
	{
		// If there is no alphatesting we don't need a pixel-shader
		newShader = NULL;
	}

	// Bind, if changed
	if(active != newShader)
	{
		ActivePS = newShader;
		if(ActivePS)
		{
			ActivePS->Apply();
		}
		else
		{
			// Unbind pixelshader
			Context->PSSetShader(NULL, NULL, 0);
		}
	}
}

/** Binds the right shader for the given texture */
void D3D11GraphicsEngineTest::BindShaderForTextureDiffusePass(zCTexture* texture)
{
	D3D11PShader* active = ActivePS;
	D3D11PShader* newShader = ActivePS;

	// Alphatest not needed for second pass since the depthbuffer is already filled, go straight for diffuse
	if(texture->GetSurface()->GetNormalmap())
	{
		if(texture->GetSurface()->GetFxMap())
		{
			newShader = PS_DiffuseNormalmappedFxMap;
		}else
		{
			newShader = PS_DiffuseNormalmapped;
		}
	}else
	{
		newShader = PS_Diffuse;
	}


	// Bind, if changed
	if(active != newShader)
	{
		ActivePS = newShader;
		ActivePS->Apply();
	}
}

/** Binds the PNAEN-Tesselation shaders if possible. Removes them from the pipeline otherwise */
bool D3D11GraphicsEngineTest::TryBindPNAENShaderForVisual(MeshVisualInfo* visual, MeshInfo* mesh)
{
	// Check if the input-mesh supports tesselation and has it enabled
	// Also don't enable tesselation during Shadow- or Reflection-Passes
	if(	!mesh->IndicesPNAEN.empty() && 
		(RenderingStage == DES_MAIN || RenderingStage == DES_Z_PRE_PASS) && 
		visual->TesselationInfo.buffer.VT_TesselationFactor > 0.0f)
	{
		Setup_PNAEN(PNAEN_Instanced);
		visual->TesselationInfo.Constantbuffer->BindToDomainShader(1);
		visual->TesselationInfo.Constantbuffer->BindToHullShader(1);
	}else if(ActiveHDS) // Remove bound tesselation shaders if we don't want them
	{
		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->DSSetShader(NULL, NULL, NULL);
		Context->HSSetShader(NULL, NULL, NULL);
		ActiveHDS = NULL;
		SetActiveVertexShader("VS_ExInstancedObj");
		ActiveVS->Apply();
	}

	return XR_SUCCESS;
}

/** Fills the Instancing-Buffer with the collected visible vobs */
XRESULT D3D11GraphicsEngineTest::FillInstancingBuffer(const std::vector<VobInfo *>& vobs)
{
	D3D11_BUFFER_DESC desc;
	((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

	// Check size of instancing buffer. Increase if needed.
	if(desc.ByteWidth < sizeof(VobInstanceInfo) * vobs.size())
	{
		LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << sizeof(VobInstanceInfo) * vobs.size() << " bytes. Recreating buffer.";

		// Buffer too small, recreate it
		delete DynamicInstancingBuffer;
		DynamicInstancingBuffer = new D3D11VertexBuffer();
		DynamicInstancingBuffer->Init(NULL, sizeof(VobInstanceInfo) * vobs.size(), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	// Map-Information of the buffer
	byte* data;
	UINT size;

	// Current instance index location
	UINT loc = 0;

	// Get reference to visual-map
	std::hash_map<zCProgMeshProto*, MeshVisualInfo*> vis = Engine::GAPI->GetStaticMeshVisuals();

	// Map the buffer, don't care about its previous content
	if(XR_SUCCESS == DynamicInstancingBuffer->Map(BaseVertexBuffer::M_WRITE_DISCARD, (void**)&data, &size))
	{
		// Check every visual for instances and add the instances to the buffer
		for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
			if(!(*it).second->Instances.empty())
			{
				// This is the starting offset of the buffer for this visual.
				// Saves us binding different smaller buffers over and over again
				(*it).second->StartInstanceNum = loc;

				// Copy instances of thiss visual
				memcpy(data + loc * sizeof(VobInstanceInfo), &(*it).second->Instances[0], sizeof(VobInstanceInfo) * (*it).second->Instances.size());

				// Increase base index for the next visual
				loc += (*it).second->Instances.size();
			}
		}
		DynamicInstancingBuffer->Unmap();
	}

	return XR_SUCCESS;
}

/** Marks all given vobs as not visible in the current frame */
void D3D11GraphicsEngineTest::MarkVobNonVisibleInFrame(const std::vector<VobInfo *>& vobs)
{
	for(unsigned int i=0;i<vobs.size();i++)
	{
		vobs[i]->VisibleInRenderPass = false;
	}
}

/** Binds the distance-buffer for the given visual.
Needed for smooth fade-out in the distance */
void D3D11GraphicsEngineTest::BindDistanceInformationFor(MeshVisualInfo* visual, int slot)
{
	if(!visual)
	{
		// Bind the infinite buffer in this case
		InfiniteRangeConstantBuffer->BindToPixelShader(slot);
		return;
	}

	// Bind the right distance buffer for this visual
	if(visual->MeshSize < Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize)
	{
		OutdoorSmallVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius - visual->MeshSize, 0, 0, 0));
		OutdoorSmallVobsConstantBuffer->BindToPixelShader(slot);
	}
	else
	{
		OutdoorVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius - visual->MeshSize, 0, 0, 0));
		OutdoorVobsConstantBuffer->BindToPixelShader(slot);
	}
}