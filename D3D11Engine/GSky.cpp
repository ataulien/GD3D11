#include "pch.h"
#include "GSky.h"
#include "zCTimer.h"
#include "zCSkyController_Outdoor.h"
#include "oCGame.h"
#include "BaseGraphicsEngine.h"
#include "zCMaterial.h"
#include "zCWorld.h"
#include "zCMesh.h"
#include "GMesh.h"
#include "BaseTexture.h"
#include "Engine.h"

GSky::GSky(void)
{
	SkyPlaneVertexBuffer = NULL;
	SkyDome = NULL;
	SkyPlane = NULL;
	CloudTexture = NULL;

	Atmosphere.Kr = 0.0045f;
	Atmosphere.Km = 0.0010f;
	Atmosphere.ESun = 20.0f;
	Atmosphere.InnerRadius = 800000;
	Atmosphere.OuterRadius = 900000;
	Atmosphere.Samples = 3;
	Atmosphere.RayleightScaleDepth = 0.25f;
	Atmosphere.G = -0.98f;
	//Atmosphere.WaveLengths = float3(0.65f, 0.57f, 0.475f);
	Atmosphere.WaveLengths = float3(0.63f, 0.57f, 0.50f);
	Atmosphere.SpherePosition = D3DXVECTOR3(0,0,0);
	Atmosphere.LightDirection = D3DXVECTOR3(1,1,1);
	Atmosphere.SphereOffsetY = -820000;
	Atmosphere.SkyTimeScale = 1.0f;
	D3DXVec3Normalize(&Atmosphere.LightDirection, &Atmosphere.LightDirection);

	ZeroMemory(&AtmosphereCB, sizeof(AtmosphereCB));
}


GSky::~GSky(void)
{
	delete SkyPlaneVertexBuffer;
	delete SkyDome;
	delete SkyPlane;
	delete CloudTexture;
	delete NightTexture;

	for(unsigned int i=0;i<SkyTextures.size();i++)
		delete SkyTextures[i];

	for(unsigned int i=0;i<CloudMeshes.size();i++)
		delete CloudMeshes[i];
}

/** Creates needed resources by the sky */
XRESULT GSky::InitSky()
{
	const float sizeX = 500000;
	const float sizeY = 10000;


	/*SkyPlaneVertices[0].Position = float3(+sizeX, sizeY, -sizeX); // 1,0
	SkyPlaneVertices[1].Position = float3(+sizeX, sizeY, +sizeX); // 1,1
	SkyPlaneVertices[2].Position = float3(-sizeX, sizeY, +sizeX); // 0,1
	SkyPlaneVertices[3].Position = float3(-sizeX, sizeY, -sizeX); // 0,0*/

	SkyPlaneVertices[0].Position = float3(-sizeX, sizeY, -sizeX); // 0
	SkyPlaneVertices[1].Position = float3(+sizeX, sizeY, -sizeX); // 1
	SkyPlaneVertices[2].Position = float3(-sizeX, sizeY, +sizeX); // 2

	SkyPlaneVertices[3].Position = float3(+sizeX, sizeY, -sizeX); // 1
	SkyPlaneVertices[4].Position = float3(+sizeX, sizeY, +sizeX); // 3
	SkyPlaneVertices[5].Position = float3(-sizeX, sizeY, +sizeX); // 2

	const float scale = 20.0f;
	D3DXVECTOR2 displacement = D3DXVECTOR2(0,0);
	float4 color = float4(1,1,1,1);

	// Construct vertices
	// 0
	SkyPlaneVertices[0].TexCoord = float2(displacement);
	SkyPlaneVertices[0].Color = color.ToDWORD();

	// 1
	SkyPlaneVertices[1].TexCoord = float2(D3DXVECTOR2(scale, 0) + displacement);
	SkyPlaneVertices[1].Color = color.ToDWORD();

	// 2
	SkyPlaneVertices[2].TexCoord = float2(D3DXVECTOR2(0, scale) + displacement);
	SkyPlaneVertices[2].Color = color.ToDWORD();

	// ---

	// 1
	SkyPlaneVertices[3].TexCoord = float2(D3DXVECTOR2(scale, 0) + displacement);
	SkyPlaneVertices[3].Color = color.ToDWORD();

	// 3
	SkyPlaneVertices[4].TexCoord = float2(D3DXVECTOR2(scale, scale) + displacement);
	SkyPlaneVertices[4].Color = color.ToDWORD();

	// 2
	SkyPlaneVertices[5].TexCoord = float2(D3DXVECTOR2(0, scale) + displacement);
	SkyPlaneVertices[5].Color = color.ToDWORD();

	return XR_SUCCESS;
}

/** Returns the cloud meshes */
std::vector<GMesh *>& GSky::GetCloudMeshes()
{
	return CloudMeshes;
}

/** Returns the skyplane */
MeshInfo* GSky::GetSkyPlane()
{
	return SkyPlane;
}


/** Adds a sky texture. Sky textures must be in order to make the daytime work */
XRESULT GSky::AddSkyTexture(const std::string& file)
{
	BaseTexture* t;
	XLE(Engine::GraphicsEngine->CreateTexture(&t));
	XLE(t->Init(file));

	SkyTextures.push_back(t);

	return XR_SUCCESS;
}

/** Loads the sky resources */
XRESULT GSky::LoadSkyResources()
{
	SkyDome = new GMesh;
	SkyDome->LoadMesh("system\\GD3D11\\meshes\\unitSphere.obj");
	//SkyDome->LoadMesh("system\\GD3D11\\meshes\\skySphere.obj");

	//AddCloudMesh("system\\GD3D11\\meshes\\cloud0.obj");
	//AddCloudMesh("system\\GD3D11\\meshes\\cloud1.obj");

	LogInfo() << "Loading sky textures...";
	//AddSkyTexture("system\\GD3D11\\textures\\sky\\cgcookie_sky01.dds");
	/*AddSkyTexture("system\\GD3D11\\textures\\sky\\cgcookie_sky01.dds");
	AddSkyTexture("system\\GD3D11\\textures\\sky\\cgcookie_sky02.dds");
	AddSkyTexture("system\\GD3D11\\textures\\sky\\cgcookie_sky03.dds");
	AddSkyTexture("system\\GD3D11\\textures\\sky\\cgcookie_sky04.dds");
	AddSkyTexture("system\\GD3D11\\textures\\sky\\cgcookie_sky05.dds");*/

	
	XLE(Engine::GraphicsEngine->CreateTexture(&CloudTexture));

#ifdef BUILD_GOTHIC_1_08k
	XLE(CloudTexture->Init("system\\GD3D11\\Textures\\SkyDay_G1.dds"));
#else
	XLE(CloudTexture->Init("system\\GD3D11\\Textures\\SkyDay.dds"));
#endif

	XLE(Engine::GraphicsEngine->CreateTexture(&NightTexture));
	XLE(NightTexture->Init("system\\GD3D11\\Textures\\SkyNight.dds"));

	VERTEX_INDEX indices[] = {0,1,2,3,4,5};
	SkyPlane = new MeshInfo;
	SkyPlane->Create(SkyPlaneVertices, 6, indices, 6);

	return XR_SUCCESS;
}

/** Adds a cloud mesh */
XRESULT GSky::AddCloudMesh(const std::string& file)
{
	GMesh* cm = new GMesh;
	cm->LoadMesh(file);
	CloudMeshes.push_back(cm);

	return XR_SUCCESS;
}

/** Returns the sky-texture for the passed daytime (0..1) */
void GSky::GetTextureOfDaytime(float time, BaseTexture** t1, BaseTexture** t2, float* factor)
{
	if(!SkyTextures.size())
		return;

	time -= floor(time); // Fractionalize, put into 0..1 range

	// Get the index of the current texture
	float index = time * (SkyTextures.size() - 0.5f);

	// Get indices of the current and the next texture
	int i0 = (int)index;
	int i1 = (unsigned int)index + 1 < SkyTextures.size() ? (int)index + 1 : 0;

	// Calculate weight
	float weight = index - (i0);

	*t1 = SkyTextures[i0];
	*t2 = SkyTextures[i1];
	*factor = weight;

}

/** Renders the sky */
XRESULT GSky::RenderSky()
{
	if(!SkyDome)
	{
		XLE(LoadSkyResources());
	}

	//return XR_SUCCESS;

	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	D3DXVECTOR3 LightDir;

	if(Engine::GAPI->GetRendererState()->RendererSettings.ReplaceSunDirection)
	{
		LightDir = Atmosphere.LightDirection;
	}else
	{
		zCSkyController_Outdoor* sc = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();
		if(sc)
		{
			LightDir = sc->GetSunWorldPosition(Atmosphere.SkyTimeScale);
			Atmosphere.LightDirection = LightDir;
		}
	}
	D3DXVec3Normalize(&LightDir, &LightDir);

	//Atmosphere.SpherePosition.y = -Atmosphere.InnerRadius;

	Atmosphere.SpherePosition.x = 0;//Engine::GAPI->GetLoadedWorldInfo()->MidPoint.x;
	Atmosphere.SpherePosition.z = 0;//Engine::GAPI->GetLoadedWorldInfo()->MidPoint.y;
	Atmosphere.SpherePosition.y = 0;//Engine::GAPI->GetLoadedWorldInfo()->LowestVertex - Atmosphere.InnerRadius;

	D3DXVECTOR3 sp = camPos;
	sp.y += Atmosphere.SphereOffsetY;

	// Fill atmosphere buffer for this frame
	AtmosphereCB.AC_CameraPos = D3DXVECTOR3(0, -Atmosphere.SphereOffsetY, 0);
	AtmosphereCB.AC_Time = Engine::GAPI->GetTimeSeconds();
	AtmosphereCB.AC_LightPos = LightDir;
	AtmosphereCB.AC_CameraHeight = -Atmosphere.SphereOffsetY;
	AtmosphereCB.AC_InnerRadius = Atmosphere.InnerRadius;
	AtmosphereCB.AC_OuterRadius = Atmosphere.OuterRadius;
	AtmosphereCB.AC_nSamples = Atmosphere.Samples;
	AtmosphereCB.AC_fSamples = (float)AtmosphereCB.AC_nSamples;

	AtmosphereCB.AC_Kr4PI = Atmosphere.Kr * 4 * (float)D3DX_PI;
	AtmosphereCB.AC_Km4PI = Atmosphere.Km * 4 * (float)D3DX_PI;
	AtmosphereCB.AC_KrESun = Atmosphere.Kr * Atmosphere.ESun;
	AtmosphereCB.AC_KmESun = Atmosphere.Km * Atmosphere.ESun;

	AtmosphereCB.AC_Scale = 1.0f / (AtmosphereCB.AC_OuterRadius - AtmosphereCB.AC_InnerRadius);
	AtmosphereCB.AC_RayleighScaleDepth = Atmosphere.RayleightScaleDepth;
	AtmosphereCB.AC_RayleighOverScaleDepth = AtmosphereCB.AC_Scale / AtmosphereCB.AC_RayleighScaleDepth;
	AtmosphereCB.AC_g = Atmosphere.G;
	AtmosphereCB.AC_Wavelength = Atmosphere.WaveLengths;
	AtmosphereCB.AC_SpherePosition = sp;

	//Engine::GraphicsEngine->DrawSky();

	// Extract fog settings
	/*zCSkyController_Outdoor* sky = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();
	Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor = float3(sky->GetMasterState()->FogColor / 255.0f);
	Engine::GAPI->GetRendererState()->GraphicsState.FF_FogNear = 0.3f * sky->GetMasterState()->FogDist; // That 0.3f is hardcoded in gothic
	Engine::GAPI->GetRendererState()->GraphicsState.FF_FogFar = sky->GetMasterState()->FogDist;
	*/
	return XR_SUCCESS;
}

/** Returns the loaded sky-Dome */
GMesh* GSky::GetSkyDome()
{
	return SkyDome;
}

/** Renders the sky */
/*XRESULT GSky::RenderSky()
{
zCSkyController_Outdoor* sky = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();

if(!sky)
return XR_SUCCESS; // Happens on levelstart

// Do sky-logic
if(!sky->GetInitDone())
{
sky->Init();
}else
{
sky->Interpolate();
}

zCSkyPlanet* sun = sky->GetSun();

zCSkyLayer* layers = sky->GetSkyLayers(0);
zCSkyState** states = sky->GetSkyLayerStates();

if(!states)
return XR_SUCCESS; // Happens on levelstart

zCSkyState* state1 = sky->GetSkyState(1);
zCSkyState* state0 = sky->GetSkyState(0);

if(state0)RenderSkyLayer(sky->GetSkyLayers(0), state0);
if(state1)RenderSkyLayer(sky->GetSkyLayers(1), state1);

return XR_SUCCESS;
}*/

/** Renders a sky layer */
XRESULT GSky::RenderSkyLayer(zCSkyLayer* layer, zCSkyState* state)
{
	/*zCSkyController_Outdoor* sky = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();
	float time = zCTimer::GetTimer()->totalTimeFloat;
	float4 color = float4(layer->GetSkyColor());

	// Get texture displacement for the current time
	D3DXVECTOR2 displacement;
	if(layer->IsNight())
	displacement = D3DXVECTOR2(0.0f,0.0f);
	else
	displacement = state->Layer[layer->GetLayerChannel()].TexSpeed * time * 0.00001f;

	// Gothic wants to place the texturecoords in 0..1 range
	displacement.x -= floor(displacement.x);
	displacement.y -= floor(displacement.y);

	const float scale = 20.0f * state->Layer[layer->GetLayerChannel()].TexScale;

	// Construct vertices
	// 0
	SkyPlaneVertices[0].TexCoord = float2(displacement);
	SkyPlaneVertices[0].Color = color.ToDWORD();

	// 1
	SkyPlaneVertices[1].TexCoord = float2(D3DXVECTOR2(scale, 0) + displacement);
	SkyPlaneVertices[1].Color = color.ToDWORD();

	// 2
	SkyPlaneVertices[2].TexCoord = float2(D3DXVECTOR2(scale, scale) + displacement);
	SkyPlaneVertices[2].Color = color.ToDWORD();

	// ---

	// 1
	SkyPlaneVertices[3].TexCoord = float2(D3DXVECTOR2(scale, 0) + displacement);
	SkyPlaneVertices[3].Color = color.ToDWORD();

	// 3
	SkyPlaneVertices[4].TexCoord = float2(D3DXVECTOR2(0, scale) + displacement);
	SkyPlaneVertices[4].Color = color.ToDWORD();

	// 2
	SkyPlaneVertices[5].TexCoord = float2(D3DXVECTOR2(scale, scale) + displacement);
	SkyPlaneVertices[5].Color = color.ToDWORD();

	// Create a rotaion only view-matrix
	D3DXMATRIX invView;
	oCGame::GetGame()->_zCSession_camVob->GetWorldMatrix(&invView);

	// Remove translation
	invView(0,3) = 0;
	invView(1,3) = 0;
	invView(2,3) = 0;

	D3DXMatrixInverse(&invView, NULL, &invView);

	// Set matrix
	Engine::GAPI->SetWorldTransform(invView);

	if(!layer->SkyPoly)
	return XR_SUCCESS;

	// Get the layers material
	zCMaterial* material;
	if(layer->SkyMode == 0) // Poly sky
	{
	material = layer->SkyPoly->GetMaterial();

	if(material)
	{
	if(!material->GetTexture())
	return XR_SUCCESS;

	material->BindTexture(0);
	}
	}else // Dome sky
	{
	material = layer->SkyDomeMesh->GetPolygons()[0].GetMaterial();
	if(material)
	{
	if(!material->GetTexture())
	return XR_SUCCESS;

	material->BindTexture(0);
	}
	}



	//zSTRING* str = (zSTRING*)state->Layer[layer->GetLayerChannel()].zSTring_TexName;
	//LogInfo() << str->ToChar();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	//Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = false;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	if(!SkyPlaneVertexBuffer)
	{
	Engine::GraphicsEngine->CreateVertexBuffer(&SkyPlaneVertexBuffer);
	SkyPlaneVertexBuffer->Init(NULL, 6 * sizeof(ExVertexStruct), BaseVertexBuffer::EBindFlags::B_VERTEXBUFFER, BaseVertexBuffer::EUsageFlags::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	SkyPlaneVertexBuffer->UpdateBuffer(SkyPlaneVertices);
	Engine::GraphicsEngine->DrawVertexBuffer(SkyPlaneVertexBuffer, 6);

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	//Engine::GAPI->DrawTriangle();
	// Draw sky plane
	//Engine::GraphicsEngine->DrawVertexArray(SkyPlaneVertices, 6);*/

	return XR_SUCCESS;
}

/** Returns the current sky-light color */
float4 GSky::GetSkylightColor()
{
	zCSkyController_Outdoor* sc = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();
	float4 color = float4(1,1,1,1);

	if(sc)
	{
		DWORD* clut = sc->PolyLightCLUTPtr;

		if(clut)
		{

		}
	}

	return color;
}

/** Returns the cloud texture */
BaseTexture* GSky::GetCloudTexture()
{
	return CloudTexture;
}

/** Returns the cloud texture */
BaseTexture* GSky::GetNightTexture()
{
	return NightTexture;
}

// The scale equation calculated by Vernier's Graphical Analysis
float AC_Escale(float fCos, float rayleighScaleDepth)
{
	float x = 1.0f - fCos;
	return rayleighScaleDepth * exp(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25f))));
}

// Calculates the Mie phase function
float AC_getMiePhase(float fCos, float fCos2, float g, float g2)
{
	return 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + fCos2) / pow(abs(1.0f + g2 - 2.0f*g*fCos), 1.5f);
}

// Calculates the Rayleigh phase function
float AC_getRayleighPhase(float fCos2)
{
	//return 1.0;
	return 0.75f + 0.75f*fCos2;
}
// Returns the near intersection point of a line and a sphere
float AC_getNearIntersection(D3DXVECTOR3 v3Pos, D3DXVECTOR3 v3Ray, float fDistance2, float fRadius2)
{
	float B = 2.0f * D3DXVec3Dot(&v3Pos, &v3Ray);
	float C = fDistance2 - fRadius2;
	float fDet = std::max(0.0f, B*B - 4.0f * C);
	return 0.5f * (-B - sqrt(fDet));
}
// Returns the far intersection point of a line and a sphere
float AC_getFarIntersection(D3DXVECTOR3 v3Pos, D3DXVECTOR3 v3Ray, float fDistance2, float fRadius2)
{
	float B = 2.0f * D3DXVec3Dot(&v3Pos, &v3Ray);
	float C = fDistance2 - fRadius2;
	float fDet = std::max(0.0f, B*B - 4.0f * C);
	return 0.5f * (-B + sqrt(fDet));
}

/** Returns the current sun color */
float3 GSky::GetSunColor()
{
	D3DXVECTOR3 wPos = ((*AtmosphereCB.AC_LightPos.toD3DXVECTOR3()) * AtmosphereCB.AC_OuterRadius);
	wPos += D3DXVECTOR3(Atmosphere.SpherePosition.x, 
						Atmosphere.SpherePosition.y + Atmosphere.SphereOffsetY, 
						Atmosphere.SpherePosition.z);


	D3DXVECTOR3 camPos = *AtmosphereCB.AC_CameraPos.toD3DXVECTOR3();
	D3DXVECTOR3 vPos = wPos - (*AtmosphereCB.AC_SpherePosition.toD3DXVECTOR3());
	D3DXVECTOR3 vRay = vPos - camPos;
				
	float fFar = D3DXVec3Length(&vRay);
	vRay /= fFar;
	
	//return float4(abs(AC_SpherePosition), 1);
	
	//if(AC_CameraHeight < AC_InnerRadius)
	//	return float4(1,0,0,1);
	
	// Calculate the closest intersection of the ray with the outer atmosphere (which is the near point of the ray passing through the atmosphere)
	float fNear = AC_getNearIntersection(camPos, vRay, AtmosphereCB.AC_CameraHeight * AtmosphereCB.AC_CameraHeight, AtmosphereCB.AC_OuterRadius * AtmosphereCB.AC_OuterRadius);

	// Calculate the ray's starting position, then calculate its scattering offset
	D3DXVECTOR3 vStart = camPos;

	float fHeight = D3DXVec3Length(&vStart);
	float fDepth = exp(AtmosphereCB.AC_RayleighOverScaleDepth * (AtmosphereCB.AC_InnerRadius - AtmosphereCB.AC_CameraHeight));
	float fStartAngle = D3DXVec3Dot(&vRay, &vStart) / fHeight;
	float fStartOffset = fDepth*AC_Escale(fStartAngle, AtmosphereCB.AC_RayleighScaleDepth);
	
	// Initialize the scattering loop variables
	float fSampleLength = fFar / AtmosphereCB.AC_fSamples;
	float fScaledLength = fSampleLength * AtmosphereCB.AC_Scale;
	D3DXVECTOR3 vSampleRay = vRay * fSampleLength;
	D3DXVECTOR3 vSamplePoint = vStart + vSampleRay * 0.5f;
	
	D3DXVECTOR3 vInvWavelength = D3DXVECTOR3(1.0f / pow(AtmosphereCB.AC_Wavelength.x, 4.0f),
		1.0f / pow(AtmosphereCB.AC_Wavelength.y, 4.0f),
		1.0f / pow(AtmosphereCB.AC_Wavelength.z, 4.0f));
	
	//return retF(AC_InnerRadius - length(vSamplePoint));
	
	// Now loop through the sample rays
	D3DXVECTOR3 vFrontColor = D3DXVECTOR3(0.0, 0.0, 0.0);
	for(int i=0; i<AtmosphereCB.AC_nSamples; i++)
	{
		float fHeight = D3DXVec3Length(&vSamplePoint);
		float fDepth = exp(AtmosphereCB.AC_RayleighOverScaleDepth * (AtmosphereCB.AC_InnerRadius - fHeight));
		float fLightAngle = D3DXVec3Dot(AtmosphereCB.AC_LightPos.toD3DXVECTOR3(), &vSamplePoint) / fHeight;
		float fCameraAngle = D3DXVec3Dot(&vRay, &vSamplePoint) / fHeight;
		float fScatter = (fStartOffset + fDepth*(AC_Escale(fLightAngle, AtmosphereCB.AC_RayleighScaleDepth) - AC_Escale(fCameraAngle, AtmosphereCB.AC_RayleighScaleDepth)));
		
		D3DXVECTOR3 vAttenuate;
		vAttenuate.x = exp(-fScatter * (vInvWavelength.x * AtmosphereCB.AC_Kr4PI + AtmosphereCB.AC_Km4PI));
		vAttenuate.y = exp(-fScatter * (vInvWavelength.y * AtmosphereCB.AC_Kr4PI + AtmosphereCB.AC_Km4PI));
		vAttenuate.z = exp(-fScatter * (vInvWavelength.z * AtmosphereCB.AC_Kr4PI + AtmosphereCB.AC_Km4PI));
		
		vFrontColor += vAttenuate * (fDepth * fScaledLength);
		vSamplePoint += vSampleRay;
	}
	
	// Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
	D3DXVECTOR3 c0;
	c0.x = vFrontColor.x * (vInvWavelength.x * AtmosphereCB.AC_KrESun);
	c0.y = vFrontColor.y * (vInvWavelength.y * AtmosphereCB.AC_KrESun);
	c0.z = vFrontColor.z * (vInvWavelength.z * AtmosphereCB.AC_KrESun);
	
	D3DXVECTOR3 c1 = vFrontColor * AtmosphereCB.AC_KmESun;	
	D3DXVECTOR3 vDirection = camPos - vPos;
	
	float fCos = D3DXVec3Dot(AtmosphereCB.AC_LightPos.toD3DXVECTOR3(), &vDirection) / D3DXVec3Length(&vDirection);
	
	float fCos2 = fCos*fCos;

	return AC_getMiePhase(fCos, fCos2, AtmosphereCB.AC_g, AtmosphereCB.AC_g * AtmosphereCB.AC_g) * c1;
}