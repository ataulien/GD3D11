#include "pch.h"
#include "D3D11Effect.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "D3D11ShaderManager.h"
#include "GothicAPI.h"
#include "D3D11VertexBuffer.h"
#include "D3D11VShader.h"
#include "D3D11GShader.h"
#include "D3D11PShader.h"
#include "D3D11ConstantBuffer.h"
#include "GSky.h"
#include <D3DX9.h>
#include <D3DX11.h>
#include <D3DX11tex.h>
#include "RenderToTextureBuffer.h"

// TODO: Remove this!
#include "D3D11GraphicsEngine.h"

D3D11Effect::D3D11Effect(void)
{
	RainBufferDrawFrom = NULL;
	RainBufferStreamTo = NULL;
	RainBufferInitial = NULL;
	RainShadowmap = NULL;
	RainTextureArray = NULL;
	RainTextureArraySRV = NULL;
}


D3D11Effect::~D3D11Effect(void)
{
	if(RainTextureArray)RainTextureArray->Release();
	if(RainTextureArraySRV)RainTextureArraySRV->Release();

	delete RainBufferInitial;
	delete RainBufferDrawFrom;
	delete RainBufferStreamTo;
	delete RainShadowmap;
}

/** Loads a texturearray. Use like the following: Put path and prefix as parameter. The files must then be called name_xxxx.dds */
HRESULT LoadTextureArray( ID3D11Device* pd3dDevice, ID3D11DeviceContext* context, char* sTexturePrefix, int iNumTextures, ID3D11Texture2D** ppTex2D, ID3D11ShaderResourceView** ppSRV);

/** Fills a vector of random raindrop data */
void D3D11Effect::FillRandomRaindropData(std::vector<ParticleInstanceInfo>& data)
{
	/** Base taken from Nvidias Rain-Sample **/

	float radius = Engine::GAPI->GetRendererState()->RendererSettings.RainRadiusRange;
	float height = Engine::GAPI->GetRendererState()->RendererSettings.RainHeightRange;

	for(int i=0;i<data.size();i++)
	{
		ParticleInstanceInfo raindrop;
		//use rejection sampling to generate random points inside a circle of radius 1 centered at 0,0
		float SeedX;
		float SeedZ;
		bool pointIsInside = false;
		while(!pointIsInside)
		{ 
			SeedX = Toolbox::frand() - 0.5f;
			SeedZ = Toolbox::frand() - 0.5f;
			if( sqrt( SeedX*SeedX + SeedZ*SeedZ ) <= 0.5f )
				pointIsInside = true;
		}
		//save these random locations for reinitializing rain particles that have fallen out of bounds
		SeedX *= radius;
		SeedZ *= radius;
		float SeedY = Toolbox::frand() * height;
		//raindrop.seed = D3DXVECTOR3(SeedX,SeedY,SeedZ); 

		//add some random speed to the particles, to prevent all the particles from following exactly the same trajectory
		//additionally, random speeds in the vertical direction ensure that temporal aliasing is minimized
		float SpeedX = 40.0f*(Toolbox::frand()/20.0f);
		float SpeedZ = 40.0f*(Toolbox::frand()/20.0f);
		float SpeedY = 40.0f*(Toolbox::frand()/10.0f); 
		raindrop.velocity = D3DXVECTOR3(SpeedX,SpeedY,SpeedZ);

		//move the rain particles to a random positions in a cylinder above the camera
		float x = SeedX + Engine::GAPI->GetCameraPosition().x;
		float z = SeedZ + Engine::GAPI->GetCameraPosition().z;
		float y = SeedY + Engine::GAPI->GetCameraPosition().y;
		raindrop.position = D3DXVECTOR3(x,y,z); 

		//get an integer between 1 and 8 inclusive to decide which of the 8 types of rain textures the particle will use
		short* s = (short*)&raindrop.drawMode;
		s[0] = int(floor(Toolbox::frand()*8 + 1));
		s[1] = int(floor(Toolbox::frand()* 0xFFFF)); // Just a random number

		//this number is used to randomly increase the brightness of some rain particles
		float intensity = 1.0f;
		float randomIncrease = Toolbox::frand();
		if( randomIncrease > 0.8f)
			intensity += randomIncrease;

		raindrop.color = float4(SeedX, SeedY, SeedZ, randomIncrease);

		float height = 30.0f;
		raindrop.scale = float2(height / 10.0f, height / 2.0f);

		data[i] = raindrop;
	}
}

/** Draws GPU-Based rain */
XRESULT D3D11Effect::DrawRain()
{
	D3D11GraphicsEngineBase* e = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	GothicRendererState& state = *Engine::GAPI->GetRendererState();

	// Get shaders
	D3D11GShader* streamOutGS = e->GetShaderManager()->GetGShader("GS_ParticleStreamOut");
	D3D11GShader* particleGS = e->GetShaderManager()->GetGShader("GS_Raindrops");
	D3D11VShader* particleAdvanceVS = e->GetShaderManager()->GetVShader("VS_AdvanceRain");
	D3D11VShader* particleVS = e->GetShaderManager()->GetVShader("VS_ParticlePointShaded");
	D3D11PShader* rainPS = e->GetShaderManager()->GetPShader("PS_Rain");

	UINT numParticles = Engine::GAPI->GetRendererState()->RendererSettings.RainNumParticles;

	static float lastRadius = state.RendererSettings.RainRadiusRange;
	static float lastHeight = state.RendererSettings.RainHeightRange;
	static bool firstFrame = true;

	// Create resources if not already done
	if(!RainBufferDrawFrom || lastHeight != state.RendererSettings.RainHeightRange 
		|| lastRadius != state.RendererSettings.RainRadiusRange)
	{
		delete RainBufferDrawFrom;
		delete RainBufferStreamTo;
		delete RainBufferInitial;

		e->CreateVertexBuffer(&RainBufferDrawFrom);
		e->CreateVertexBuffer(&RainBufferStreamTo);
		e->CreateVertexBuffer(&RainBufferInitial);

		UINT numParticles = Engine::GAPI->GetRendererState()->RendererSettings.RainNumParticles;
		std::vector<ParticleInstanceInfo> particles(numParticles);

		// Fill the vector with random raindrop data
		FillRandomRaindropData(particles);

		// Create vertexbuffers
		RainBufferInitial->Init(&particles[0], particles.size() * sizeof(ParticleInstanceInfo), (BaseVertexBuffer::EBindFlags)(BaseVertexBuffer::B_VERTEXBUFFER));
		RainBufferDrawFrom->Init(&particles[0], particles.size() * sizeof(ParticleInstanceInfo), (BaseVertexBuffer::EBindFlags)(BaseVertexBuffer::B_VERTEXBUFFER | BaseVertexBuffer::B_STREAM_OUT));
		RainBufferStreamTo->Init(&particles[0], particles.size() * sizeof(ParticleInstanceInfo), (BaseVertexBuffer::EBindFlags)(BaseVertexBuffer::B_VERTEXBUFFER | BaseVertexBuffer::B_STREAM_OUT));

		firstFrame = true;

		if(!RainTextureArray)
		{
			// Load textures...
			LogInfo() << "Loading rain-drop textures";
			LoadTextureArray(e->GetDevice(), e->GetContext(), "system\\GD3D11\\Textures\\Raindrops\\cv0_vPositive_", 370, &RainTextureArray, &RainTextureArraySRV);
		}

		if(!RainShadowmap)
		{
			const int s = 2048;
			RainShadowmap = new RenderToDepthStencilBuffer(e->GetDevice(), s, s, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
		}
	}

	lastHeight = state.RendererSettings.RainHeightRange;
	lastRadius = state.RendererSettings.RainRadiusRange;

	D3D11VertexBuffer* b = NULL;

	// Use initial-data if we don't have something in the stream-buffers yet
	if(firstFrame || state.RendererSettings.RainUseInitialSet || Engine::GAPI->IsGamePaused())
		b = (D3D11VertexBuffer *)RainBufferInitial;
	else
		b = (D3D11VertexBuffer *)RainBufferDrawFrom;

	firstFrame = false;

	ID3D11Buffer* bobjDraw = b->GetVertexBuffer();
	ID3D11Buffer* bobjStream = ((D3D11VertexBuffer *)RainBufferStreamTo)->GetVertexBuffer();
	UINT stride = sizeof(ParticleInstanceInfo);
	UINT offset = 0;

	// Bind buffer to draw from last frame
	e->GetContext()->IASetVertexBuffers(0, 1, &bobjDraw, &stride, &offset);

	// Set stream target
	e->GetContext()->SOSetTargets(1, &bobjStream, &offset);

	// Apply shaders
	particleAdvanceVS->Apply();
	streamOutGS->Apply();

	// Rendering points only
	e->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Update constantbuffer for the advance-VS
	AdvanceRainConstantBuffer acb;
	acb.AR_LightPosition = Engine::GAPI->GetSky()->GetAtmoshpereSettings().LightDirection * Engine::GAPI->GetSky()->GetAtmoshpereSettings().OuterRadius + Engine::GAPI->GetCameraPosition();
	acb.AR_FPS = state.RendererInfo.FPS;
	acb.AR_Radius = state.RendererSettings.RainRadiusRange;
	acb.AR_Height = state.RendererSettings.RainHeightRange;
	acb.AR_CameraPosition = Engine::GAPI->GetCameraPosition();
	acb.AR_GlobalVelocity = state.RendererSettings.RainGlobalVelocity;
	acb.AR_MoveRainParticles = state.RendererSettings.RainMoveParticles ? 1 : 0;

	particleAdvanceVS->GetConstantBuffer()[0]->UpdateBuffer(&acb);
	particleAdvanceVS->GetConstantBuffer()[0]->BindToVertexShader(1);
	particleAdvanceVS->GetConstantBuffer()[0]->BindToPixelShader(1);

	// Advance particle system in VS and stream out the data
	e->GetContext()->Draw(numParticles, 0);

	// Unset streamout target
	bobjStream = NULL;
	e->GetContext()->SOSetTargets(1, &bobjStream, 0);

	// Swap buffers
	std::swap(RainBufferDrawFrom, RainBufferStreamTo);

	// ---- Draw the rain ----
	e->SetDefaultStates();

	// Set alphablending

	state.BlendState.SetAlphaBlending();
	state.BlendState.SetDirty();

	// Disable depth-write
	state.DepthState.DepthWriteEnabled = false;
	state.DepthState.SetDirty();
	state.DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::DEFAULT_DEPTH_COMP_STATE;

	// Disable culling
	state.RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	state.RasterizerState.SetDirty();

	e->UpdateRenderStates();

	// Apply particle shaders
	particleVS->Apply();
	rainPS->Apply();
	particleGS->Apply();

	// Setup constantbuffers
	ParticleGSInfoConstantBuffer gcb;
	gcb.CameraPosition = Engine::GAPI->GetCameraPosition();
	gcb.PGS_RainFxWeight = Engine::GAPI->GetRainFXWeight();
	gcb.PGS_RainHeight = state.RendererSettings.RainHeightRange;

	particleGS->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
	particleGS->GetConstantBuffer()[0]->BindToGeometryShader(2);

	ParticlePointShadingConstantBuffer scb;
	scb.View = GetRainShadowmapCameraRepl().ViewReplacement;
	scb.Projection = GetRainShadowmapCameraRepl().ProjectionReplacement;
	particleVS->GetConstantBuffer()[1]->UpdateBuffer(&scb);
	particleVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	RainShadowmap->BindToVertexShader(e->GetContext(), 0);

	// Bind view/proj
	e->SetupVS_ExConstantBuffer();

	// Bind droplets
	e->GetContext()->PSSetShaderResources(0, 1, &RainTextureArraySRV);

	// Draw the vertexbuffer
	e->DrawVertexBuffer(RainBufferDrawFrom, numParticles, sizeof(ParticleInstanceInfo));

	// Reset this
	e->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	e->GetContext()->GSSetShader(NULL, 0, 0);

	e->SetDefaultStates();
	return XR_SUCCESS;
}

/** Renders the rain-shadowmap */
XRESULT D3D11Effect::DrawRainShadowmap()
{
	if(!RainShadowmap)
		return XR_SUCCESS;

	D3D11GraphicsEngine* e = (D3D11GraphicsEngine *)Engine::GraphicsEngine; // TODO: This has to be a cast to D3D11GraphicsEngineBase!
	GothicRendererState& state = *Engine::GAPI->GetRendererState();

	CameraReplacement& cr = RainShadowmapCameraRepl;

	// Get the section we are currently in
	D3DXVECTOR3 p = Engine::GAPI->GetCameraPosition();
	D3DXVECTOR3 dir; D3DXVec3Normalize(&dir, &(-Engine::GAPI->GetRendererState()->RendererSettings.RainGlobalVelocity));
	// Set the camera height to the highest point in this section
	//p.y = 0;
	p += dir * 6000.0f;

	D3DXVECTOR3 lookAt = p;
	lookAt -= dir;

	// Create shadowmap view-matrix
	D3DXMatrixLookAtLH(&cr.ViewReplacement, &p, &lookAt, &D3DXVECTOR3(0,1,0));
	D3DXMatrixTranspose(&cr.ViewReplacement, &cr.ViewReplacement);

	D3DXMatrixOrthoLH(&cr.ProjectionReplacement, RainShadowmap->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale, 
												 RainShadowmap->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale, 1, 20000.0f);
	D3DXMatrixTranspose(&cr.ProjectionReplacement, &cr.ProjectionReplacement);

	cr.PositionReplacement = p;
	cr.LookAtReplacement = lookAt;

	// Replace gothics camera
	Engine::GAPI->SetCameraReplacementPtr(&cr);

	// Make alpharef a bit more aggressive, to make trees less rain-proof

	float oldAlphaRef = Engine::GAPI->GetRendererState()->GraphicsState.FF_AlphaRef;

	Engine::GAPI->GetRendererState()->GraphicsState.FF_AlphaRef = -1.0f;

	// Bind the FF-Info to the first PS slot
	D3D11PShader* PS_Diffuse = e->GetShaderManager()->GetPShader("PS_Diffuse");
	if(PS_Diffuse)
	{
		PS_Diffuse->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
		PS_Diffuse->GetConstantBuffer()[0]->BindToPixelShader(0);
	}

	// Disable stuff like NPCs and usable things as they don't need to cast rain-shadows
	bool oldDrawSkel = Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes;
	Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes = false;

	// Draw rain-shadowmap
	e->RenderShadowmaps(p, RainShadowmap, true, false);
	

	// Restore old settings
	Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes = oldDrawSkel;
	Engine::GAPI->GetRendererState()->GraphicsState.FF_AlphaRef = oldAlphaRef;
	if(PS_Diffuse)
	{
		PS_Diffuse->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	}

	e->SetDefaultStates();

	// Restore gothics camera
	Engine::GAPI->SetCameraReplacementPtr(NULL);

	return XR_SUCCESS;
}

//--------------------------------------------------------------------------------------
// LoadTextureArray loads a texture array and associated view from a series
// of textures on disk.
//--------------------------------------------------------------------------------------
HRESULT LoadTextureArray( ID3D11Device* pd3dDevice, ID3D11DeviceContext* context, char* sTexturePrefix, int iNumTextures, ID3D11Texture2D** ppTex2D, ID3D11ShaderResourceView** ppSRV)
{
	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(D3D11_TEXTURE2D_DESC) );

	CHAR szTextureName[MAX_PATH];
	CHAR str[MAX_PATH];
	for(int i=0; i<iNumTextures; i++)
	{
		sprintf(str, "%s%.4d.dds", sTexturePrefix, i); 

		ID3D11Resource *pRes = NULL;
		D3DX11_IMAGE_LOAD_INFO loadInfo;
		ZeroMemory( &loadInfo, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
		loadInfo.Width = D3DX_FROM_FILE;
		loadInfo.Height  = D3DX_FROM_FILE;
		loadInfo.Depth  = D3DX_FROM_FILE;
		loadInfo.FirstMipLevel = 0;
		loadInfo.MipLevels = 10;
		loadInfo.Usage = D3D11_USAGE_STAGING;
		loadInfo.BindFlags = 0;
		loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		loadInfo.MiscFlags = 0;
		loadInfo.Format = DXGI_FORMAT_R8_UNORM; 
		loadInfo.Filter = D3DX11_FILTER_TRIANGLE;
		loadInfo.MipFilter = D3DX11_FILTER_TRIANGLE;

		LE(D3DX11CreateTextureFromFile( pd3dDevice, str, &loadInfo, NULL, &pRes, &hr ));
		if( pRes )
		{
			ID3D11Texture2D* pTemp;
			pRes->QueryInterface( __uuidof( ID3D11Texture2D ), (LPVOID*)&pTemp );
			pTemp->GetDesc( &desc );


			if(DXGI_FORMAT_R8_UNORM != desc.Format)   
				return false;



			if(!(*ppTex2D))
			{
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				desc.CPUAccessFlags = 0;
				desc.ArraySize = iNumTextures;
				LE(pd3dDevice->CreateTexture2D( &desc, NULL, ppTex2D));
			}


			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			for(UINT iMip=0; iMip < desc.MipLevels; iMip++)
			{
				context->Map(pTemp, iMip, D3D11_MAP_READ, 0, &mappedTex2D);

				context->UpdateSubresource(		(*ppTex2D), 
					D3D11CalcSubresource( iMip, i, desc.MipLevels ),
					NULL,
					mappedTex2D.pData,
					mappedTex2D.RowPitch,
					0 );

				context->Unmap(pTemp, iMip);
			}

			SAFE_RELEASE( pRes );
			SAFE_RELEASE( pTemp );
		}
		else
		{
			return false;
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	SRVDesc.Texture2DArray.MipLevels = desc.MipLevels;
	SRVDesc.Texture2DArray.ArraySize = iNumTextures;
	LE(pd3dDevice->CreateShaderResourceView( *ppTex2D, &SRVDesc, ppSRV ));

	return hr;
}