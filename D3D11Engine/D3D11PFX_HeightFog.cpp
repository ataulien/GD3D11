#include "pch.h"
#include "D3D11PFX_HeightFog.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"
#include "D3D11PfxRenderer.h"
#include "RenderToTextureBuffer.h"
#include "D3D11ShaderManager.h"
#include "D3D11VShader.h"
#include "D3D11PShader.h"
#include "D3D11ConstantBuffer.h"
#include "ConstantBufferStructs.h"
#include "GothicAPI.h"
#include "GSky.h"

D3D11PFX_HeightFog::D3D11PFX_HeightFog(D3D11PfxRenderer* rnd) : D3D11PFX_Effect(rnd)
{
}


D3D11PFX_HeightFog::~D3D11PFX_HeightFog(void)
{
}

/** Draws this effect to the given buffer */
XRESULT D3D11PFX_HeightFog::Render(RenderToTextureBuffer* fxbuffer)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	// Save old rendertargets
	ID3D11RenderTargetView* oldRTV = NULL;
	ID3D11DepthStencilView* oldDSV = NULL;
	engine->GetContext()->OMGetRenderTargets(1, &oldRTV, &oldDSV);

	D3D11VShader* vs = engine->GetShaderManager()->GetVShader("VS_PFX");
	D3D11PShader* hfPS = engine->GetShaderManager()->GetPShader("PS_PFX_Heightfog");

	hfPS->Apply();
	vs->Apply();

	HeightfogConstantBuffer cb;
	D3DXMatrixInverse(&cb.InvProj, NULL, &Engine::GAPI->GetProjectionMatrix());

	Engine::GAPI->GetViewMatrix(&cb.InvView);
	D3DXMatrixInverse(&cb.InvView, NULL, &cb.InvView);
	cb.CameraPosition = Engine::GAPI->GetCameraPosition();
	float NearPlane=Engine::GAPI->GetRendererState()->RendererInfo.NearPlane;
	float FarPlane=Engine::GAPI->GetRendererState()->RendererInfo.FarPlane;

	D3DXMATRIX invViewProj, view;
	Engine::GAPI->GetViewMatrix(&view);
	D3DXMatrixMultiply(&invViewProj, &Engine::GAPI->GetProjectionMatrix(), &view);
	D3DXMatrixInverse(&invViewProj, NULL, &invViewProj);

	D3DXVECTOR3 vecFrustum[4];
	/*vecFrustum[0] = D3DXVECTOR3(-1.0f, -1.0f,  0.0f); // xyz
	vecFrustum[1] = D3DXVECTOR3( 1.0f, -1.0f,  0.0f); // Xyz
	vecFrustum[2] = D3DXVECTOR3(-1.0f,  1.0f,  0.0f); // xYz
	vecFrustum[3] = D3DXVECTOR3( 1.0f,  1.0f,  0.0f); // XYz*/
	vecFrustum[0] = D3DXVECTOR3(-1.0f, -1.0f,  1.0f); // xyZ
	vecFrustum[1] = D3DXVECTOR3( 1.0f, -1.0f,  1.0f); // XyZ
	vecFrustum[2] = D3DXVECTOR3(-1.0f,  1.0f,  1.0f); // xYZ
	vecFrustum[3] = D3DXVECTOR3( 1.0f,  1.0f,  1.0f); // XYZ

	// Get world space frustum corners
	PFXVS_ConstantBuffer vcb;
	for( int i = 0; i < 4; i++ )
	{
		D3DXVec3TransformCoord( &vecFrustum[i], &vecFrustum[i], &cb.InvProj );
		D3DXVec3Normalize(&vecFrustum[i], &vecFrustum[i]);
		D3DXVec3TransformNormal( &vecFrustum[i], &vecFrustum[i], &cb.InvView );
		
	}

	/*vcb.PFXVS_FrustumCorners[0] = D3DXVECTOR4(vecFrustum[0].x, vecFrustum[0].y, vecFrustum[0].z, 0.0f);
	vcb.PFXVS_FrustumCorners[1] = D3DXVECTOR4(vecFrustum[2].x, vecFrustum[2].y, vecFrustum[2].z, 0.0f);
	vcb.PFXVS_FrustumCorners[2] = D3DXVECTOR4(vecFrustum[1].x, vecFrustum[1].y, vecFrustum[1].z, 0.0f);

	vcb.PFXVS_FrustumCorners[3] = D3DXVECTOR4(vecFrustum[3].x, vecFrustum[3].y, vecFrustum[3].z, 0.0f);
	vcb.PFXVS_FrustumCorners[4] = D3DXVECTOR4(vecFrustum[1].x, vecFrustum[1].y, vecFrustum[1].z, 0.0f);
	vcb.PFXVS_FrustumCorners[5] = D3DXVECTOR4(vecFrustum[2].x, vecFrustum[2].y, vecFrustum[2].z, 0.0f);*/

	cb.HF_GlobalDensity = Engine::GAPI->GetRendererState()->RendererSettings.FogGlobalDensity;
	cb.HF_HeightFalloff = Engine::GAPI->GetRendererState()->RendererSettings.FogHeightFalloff;
	
	float height = Engine::GAPI->GetRendererState()->RendererSettings.FogHeight;
	D3DXVECTOR3 color = *Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod.toD3DXVECTOR3();

	float fnear = 15000.0f;
	float ffar = 60000.0f;
	float secScale = Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius;

	cb.HF_WeightZNear = std::max(0.0f, WORLD_SECTION_SIZE * ((secScale - 0.5f) * 0.7f) - (ffar - fnear)); // Keep distance from original fog but scale the near-fog up to section draw distance
	cb.HF_WeightZFar = WORLD_SECTION_SIZE * ((secScale - 0.5f) * 0.8f);
	
	float atmoMax = 83200.0f; // Fixme: Calculate!	
	float atmoMin = 27799.9922f;

	cb.HF_WeightZFar = std::min(cb.HF_WeightZFar, atmoMax);
	cb.HF_WeightZNear = std::min(cb.HF_WeightZNear, atmoMin);

	if(Engine::GAPI->GetFogOverride() > 0.0f)
	{
		// Make sure the camera is inside the fog when in fog zone
		height = Toolbox::lerp(height, Engine::GAPI->GetCameraPosition().y + 10000, Engine::GAPI->GetFogOverride()); // TODO: Get this from the actual fog-distance in the fogzone!
		
		// Override fog color when in fog zone
		color = Engine::GAPI->GetFogColor();

		// Make it z-Fog
		cb.HF_HeightFalloff = Toolbox::lerp(cb.HF_HeightFalloff, 0.000001f, Engine::GAPI->GetFogOverride());

		// Turn up density
		cb.HF_GlobalDensity = Toolbox::lerp(cb.HF_GlobalDensity, cb.HF_GlobalDensity * 2, Engine::GAPI->GetFogOverride());

		// Use other fog-values for fog-zones
		float distNear = WORLD_SECTION_SIZE * ((ffar - fnear) / ffar);
		cb.HF_WeightZNear = Toolbox::lerp(cb.HF_WeightZNear, WORLD_SECTION_SIZE * 0.09f, Engine::GAPI->GetFogOverride());
		cb.HF_WeightZFar = Toolbox::lerp(cb.HF_WeightZFar, WORLD_SECTION_SIZE * 0.8, Engine::GAPI->GetFogOverride());
	}


	/*static float s_smoothHeight = Engine::GAPI->GetRendererState()->RendererSettings.FogHeight;
	static float s_smoothZF = cb.HF_WeightZFar;
	static float s_smoothZN = cb.HF_WeightZNear;

	// Fade Z-Far and z-Near
	s_smoothZF = Toolbox::lerp(s_smoothZF, cb.HF_WeightZFar, std::min(Engine::GAPI->GetFrameTimeSec() * 5.0f, 1.0f));
	s_smoothZN = Toolbox::lerp(s_smoothZN, cb.HF_WeightZNear, std::min(Engine::GAPI->GetFrameTimeSec() * 5.0f, 1.0f));

	cb.HF_WeightZNear = s_smoothZN;
	cb.HF_WeightZFar = s_smoothZF;*/

	//static D3DXVECTOR3 s_smoothColor = *Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod.toD3DXVECTOR3();

	// Fade fog height in case it changes
	//s_smoothHeight = Toolbox::lerp(s_smoothHeight, height, std::min(Engine::GAPI->GetFrameTimeSec() * 5.0f, 1.0f));

	// Fade color, because leaving a fogzone would make the fog-color pop otherwise
	//D3DXVec3Lerp(&s_smoothColor, &s_smoothColor, &color, std::min(Engine::GAPI->GetFrameTimeSec() * 5.0f, 1.0f));

	//D3DXVECTOR3 fogColorMod = Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod;
	cb.HF_FogColorMod = color;//Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod;

	cb.HF_FogHeight = height;



	//cb.HF_FogColorMod = Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor;
	cb.HF_ProjAB = float2(	Engine::GAPI->GetProjectionMatrix()._33,
							Engine::GAPI->GetProjectionMatrix()._34);

	hfPS->GetConstantBuffer()[0]->UpdateBuffer(&cb);
	hfPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	vs->GetConstantBuffer()[0]->UpdateBuffer(&vcb);
	vs->GetConstantBuffer()[0]->BindToVertexShader(0);

	engine->GetContext()->OMSetRenderTargets(1, &oldRTV, NULL);

	// Bind depthbuffer
	engine->GetDepthBuffer()->BindToPixelShader(engine->GetContext(), 1);

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	//Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	// Copy
	FxRenderer->DrawFullScreenQuad();


	// Restore rendertargets
	ID3D11ShaderResourceView* srv = NULL;
	engine->GetContext()->PSSetShaderResources(1,1,&srv);

	engine->GetContext()->OMSetRenderTargets(1, &oldRTV, oldDSV);
	if(oldRTV)oldRTV->Release();
	if(oldDSV)oldDSV->Release();

	return XR_SUCCESS;
}