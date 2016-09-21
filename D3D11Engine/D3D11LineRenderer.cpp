#include "pch.h"
#include "D3D11LineRenderer.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "D3D11VertexBuffer.h"
#include "GothicAPI.h"
#include "D3D11VertexBuffer.h"

D3D11LineRenderer::D3D11LineRenderer(void)
{
	LineBuffer = NULL;
	LineBufferSize = 0;
}


D3D11LineRenderer::~D3D11LineRenderer(void)
{
	delete LineBuffer;
}

/** Adds a line to the list */
XRESULT D3D11LineRenderer::AddLine(const LineVertex& v1, const LineVertex& v2)
{
	if(LineCache.size() >= 0xFFFFFFFF)
	{
		return XR_FAILED;
	}

	LineCache.push_back(v1);
	LineCache.push_back(v2);
	return XR_SUCCESS;
}

/** Flushes the cached lines */
XRESULT D3D11LineRenderer::Flush()
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	if(LineCache.size() == 0)
		return XR_SUCCESS;

	// Check buffersize and create a new one if needed
	if(!LineBuffer || LineCache.size() > LineBufferSize)
	{
		// Create a new buffer
		delete LineBuffer;

		XLE(engine->CreateVertexBuffer(&LineBuffer));
		XLE(LineBuffer->Init(&LineCache[0], LineCache.size() * sizeof(LineVertex), D3D11VertexBuffer::B_VERTEXBUFFER, D3D11VertexBuffer::U_DYNAMIC, D3D11VertexBuffer::CA_WRITE));
		LineBufferSize = LineCache.size();
	}else
	{
		// Just update our buffer
		XLE(LineBuffer->UpdateBuffer(&LineCache[0], LineCache.size() * sizeof(LineVertex)));
	}
	
	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);
	Engine::GAPI->SetWorldTransform(world);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	engine->SetActivePixelShader("PS_Lines");
	engine->SetActiveVertexShader("VS_Lines");

	engine->SetupVS_ExMeshDrawCall();
	engine->SetupVS_ExConstantBuffer();
	engine->SetupVS_ExPerInstanceConstantBuffer();
	engine->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	engine->SetDefaultStates();
	Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	engine->UpdateRenderStates();

	// Draw the lines
	UINT offset = 0;
	UINT uStride = sizeof(LineVertex);
	ID3D11Buffer* buffer = ((D3D11VertexBuffer *)LineBuffer)->GetVertexBuffer();
	engine->GetContext()->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	engine->GetContext()->Draw(LineCache.size(), 0);

	//engine->DrawVertexBuffer(LineBuffer, LineCache.size(), sizeof(LineVertex));

	// Clear for the next frame
	ClearCache();
	return XR_SUCCESS;
}

/** Clears the line cache */
XRESULT D3D11LineRenderer::ClearCache()
{
	LineCache.clear();
	return XR_SUCCESS;
}