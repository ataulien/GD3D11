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

	Engine::GAPI->SetFarPlane(80000.0f);
	Clear(float4(Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor, 0.0f));

	ID3D11RenderTargetView* rtvs[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
	Context->OMSetRenderTargets(2, rtvs, DepthStencilBuffer->GetDepthStencilView());

	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
		DrawWorldMeshTest();
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
		DrawVobsTest();

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

void D3D11GraphicsEngineTest::DrawWorldMeshTest()
{
	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);
	Engine::GAPI->ResetWorldTransform();

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();
	SetupVS_ExPerInstanceConstantBuffer();



	DistortionTexture->BindToPixelShader(0);

	std::list<WorldMeshSectionInfo*> renderList;
	Engine::GAPI->CollectVisibleSections(renderList);

	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, 
				Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	std::list<std::pair<MeshKey, MeshInfo *>> meshList;

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

			meshList.push_back((*itm));
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
	meshList.sort(cmpstruct::cmp);

	// Draw depth only
	Context->PSSetShader(NULL, NULL, NULL);
	for(std::list<std::pair<MeshKey, MeshInfo *>>::iterator it = meshList.begin(); it != meshList.end(); it++)
	{
		if((*it).first.Texture->HasAlphaChannel())
			continue; // Don't pre-render stuff with alpha channel

		DrawVertexBufferIndexedUINT(NULL, NULL, 
			(*it).second->Indices.size(),
			(*it).second->BaseIndexLocation);
	}

	// Now draw the actual pixels
	zCTexture* bound = NULL;
	for(std::list<std::pair<MeshKey, MeshInfo *>>::iterator it = meshList.begin(); it != meshList.end(); it++)
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

			// Get the right shader for it
			BindShaderForTexture((*it).first.Texture);

			/*MaterialInfo* info = Engine::GAPI->GetMaterialInfoFrom((*it).first.Material->GetTextureSingle());
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();

			info->Constantbuffer->BindToPixelShader(2);*/
		}

		// Draw the section-part
		DrawVertexBufferIndexedUINT(NULL, NULL, 
			(*it).second->Indices.size(),
			(*it).second->BaseIndexLocation);
	}
}

/** Draws vobs */
void D3D11GraphicsEngineTest::DrawVobsTest()
{
	START_TIMING();

	const std::hash_map<zCProgMeshProto*, MeshVisualInfo*>& vis = Engine::GAPI->GetStaticMeshVisuals();

	//SetDefaultStates();

	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_Diffuse");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_ExInstancedObj");

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	// Use default material info for now
	MaterialInfo defInfo;
	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&defInfo);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(2);

	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	INT2 camSection = WorldConverter::GetSectionOfPos(camPos);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	static std::vector<VobInfo *> vobs;
	static std::vector<VobLightInfo *> lights;
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs || 
		Engine::GAPI->GetRendererState()->RendererSettings.EnableDynamicLighting)
	{
		if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum ||
			(Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum && vobs.empty()))
			Engine::GAPI->CollectVisibleVobs(vobs, lights);
	}
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{

			/*static std::vector<VobInstanceInfo, AlignmentAllocator<VobInstanceInfo, 16> > s_InstanceData;
			for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
			{
	#ifdef BUILD_GOTHIC_1_08k // G1 has this sometimes?
				if(!(*it).second->Visual)
					continue;
	#endif

				(*it).second->StartInstanceNum = s_InstanceData.size();
				s_InstanceData.insert(s_InstanceData.end(), (*it).second->Instances.begin(), (*it).second->Instances.end());
			}*/

			//if(!s_InstanceData.empty())
			{

			/*if(s_InstanceData.size() * sizeof(VobInstanceInfo) % 16 != 0)
			{
				int d = 16 - (s_InstanceData.size() * sizeof(VobInstanceInfo) % 16); // Calculate missing bytes
				int numToAdd = d / sizeof(VobInstanceInfo); // Amount of instances we have to add (max 7)


				VobInstanceInfo zi;
				memset(&zi, 0, sizeof(zi));

				// Add missing data to make it aligned
				s_InstanceData.resize(s_InstanceData.size() + numToAdd, zi);
			}*/

			/*if(s_InstanceData.size() < 2)
			{
				VobInstanceInfo zi;
				memset(&zi, 0, sizeof(zi));

				s_InstanceData.push_back(zi);
			}*/

			// Create instancebuffer for this frame
			D3D11_BUFFER_DESC desc;
			((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

			if(desc.ByteWidth < sizeof(VobInstanceInfo) * vobs.size())
			{
				LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << sizeof(VobInstanceInfo) * vobs.size() << " bytes. Recreating buffer.";

				// Buffer too small, recreate it
				delete DynamicInstancingBuffer;
				DynamicInstancingBuffer = new D3D11VertexBuffer();
				DynamicInstancingBuffer->Init(NULL, sizeof(VobInstanceInfo) * vobs.size(), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
			}

			
				// Update the vertexbuffer
				//DynamicInstancingBuffer->UpdateBufferAligned16(&s_InstanceData[0], sizeof(VobInstanceInfo) * s_InstanceData.size());

			byte* data;
			UINT size;
			UINT loc = 0;
			DynamicInstancingBuffer->Map(BaseVertexBuffer::M_WRITE_DISCARD, (void**)&data, &size);
				static std::vector<VobInstanceInfo, AlignmentAllocator<VobInstanceInfo, 16> > s_InstanceData;
				for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
				{
					(*it).second->StartInstanceNum = loc;
					memcpy(data + loc * sizeof(VobInstanceInfo), &(*it).second->Instances[0], sizeof(VobInstanceInfo) * (*it).second->Instances.size());
					loc += (*it).second->Instances.size();
				}
			DynamicInstancingBuffer->Unmap();

			for(unsigned int i=0;i<vobs.size();i++)
			{
				vobs[i]->VisibleInRenderPass = false; // Reset this for the next frame
				//RenderedVobs.push_back(vobs[i]);
			}

			// Reset buffer
			s_InstanceData.resize(0);

			for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
			{
				if((*it).second->Instances.empty())
					continue;

				if((*it).second->MeshSize < Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize)
				{
					OutdoorSmallVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius - (*it).second->MeshSize, 0, 0, 0));
					OutdoorSmallVobsConstantBuffer->BindToPixelShader(3);
				}
				else
				{
					OutdoorVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius - (*it).second->MeshSize, 0, 0, 0));
					OutdoorVobsConstantBuffer->BindToPixelShader(3);
				}

				for(std::map<MeshKey, std::vector<MeshInfo*>>::iterator itt = (*it).second->MeshesByTexture.begin(); itt != (*it).second->MeshesByTexture.end(); itt++)
				{
					std::vector<MeshInfo *>& mlist = (*it).second->MeshesByTexture[(*itt).first];
					if(mlist.empty())
						continue;

					for(unsigned int i=0;i<mlist.size();i++)
					{
						zCTexture* tx = (*itt).first.Texture;

						if(!tx)
							continue;

						// Bind texture

						if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
						{
							MyDirectDrawSurface7* surface = tx->GetSurface();
							ID3D11ShaderResourceView* srv[2];
			
							// Get diffuse and normalmap
							srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
							srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;

							// Bind both
							Context->PSSetShaderResources(0,2, srv);


							// Force alphatest on vobs for now
							BindShaderForTexture(tx, true);

							MaterialInfo* info = (*itt).first.Info;
							if(!info->Constantbuffer)
								info->UpdateConstantbuffer();

							info->Constantbuffer->BindToPixelShader(2);

							if(!info->TesselationShaderPair.empty())
							{
								// Set normal/displacement map
								Context->DSSetShaderResources(0,1, &srv[1]);
								Context->HSSetShaderResources(0,1, &srv[1]);
								Setup_PNAEN(PNAEN_Instanced);
							}else if(ActiveHDS)
							{
								Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
								Context->DSSetShader(NULL, NULL, NULL);
								Context->HSSetShader(NULL, NULL, NULL);
								ActiveHDS = NULL;
							}

						}
						else
						{
#ifndef PUBLIC_RELEASE
							for(int s=0;s<(*it).second->Instances.size();s++)
							{
								D3DXVECTOR3 pos = D3DXVECTOR3((*it).second->Instances[s].world._14, (*it).second->Instances[s].world._24, (*it).second->Instances[s].world._34); 
								GetLineRenderer()->AddAABBMinMax(pos - (*it).second->BBox.Min, pos + (*it).second->BBox.Max, D3DXVECTOR4(1,0,0,1));
							}
#endif
							//continue;
						}

						MeshInfo* mi = mlist[i];

						
						// Draw batch
						DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), (int)(*it).second->Instances.size(), sizeof(ExVertexStruct), (*it).second->StartInstanceNum);
						//DrawVertexBufferIndexed((*it).second->FullMesh->MeshVertexBuffer, (*it).second->FullMesh->MeshIndexBuffer, 0);

						//Engine::GraphicsEngine->DrawVertexBufferIndexed(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size());
					}

				}

				// Reset visual
				if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum)
				{
					(*it).second->StartNewFrame();
				}
			}
		}
	}

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->DSSetShader(NULL, NULL, NULL);
	Context->HSSetShader(NULL, NULL, NULL);

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	vobs.clear();
	lights.clear();

	STOP_TIMING(GothicRendererTiming::TT_Vobs);
}