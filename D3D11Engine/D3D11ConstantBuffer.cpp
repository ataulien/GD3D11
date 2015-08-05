#include "pch.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "GothicAPI.h"

D3D11ConstantBuffer::D3D11ConstantBuffer(int size, void* data) : BaseConstantBuffer(size, data)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	char* dd = (char *)data;
	
	if(!dd)
	{
		dd= new char[size];
		ZeroMemory(dd, size);
	}

	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = dd;
	d.SysMemPitch = 0;
	d.SysMemSlicePitch = 0;
	
	// Create constantbuffer
	HRESULT hr;
	LE(engine->GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), &d, &Buffer));

	if(!data)
		delete[] dd;

	BufferDirty = false;
}


D3D11ConstantBuffer::~D3D11ConstantBuffer(void)
{
	if (Buffer)Buffer->Release();
}

/** Updates the buffer */
void D3D11ConstantBuffer::UpdateBuffer(void* data)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	//engine->GetContext()->UpdateSubresource(Buffer, 0, nullptr, data, 0, 0);
	
#ifndef PUBLIC_RELEASE
	if(GetCurrentThreadId() != Engine::GAPI->GetMainThreadID())
		LogWarn() << "UpdateBuffer called from worker-thread! Please use UpdateBufferDeferred!";
#endif

	D3D11_MAPPED_SUBRESOURCE res;
	if(XR_SUCCESS == engine->GetContext()->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res))
	{
		// Copy data
		memcpy(res.pData, data, res.DepthPitch);

		engine->GetContext()->Unmap(Buffer, 0);

		BufferDirty = true;
	}

	
}

/** Updates the buffer, threadsave */
void D3D11ConstantBuffer::UpdateBufferDeferred(void* data)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	//engine->GetContext()->UpdateSubresource(Buffer, 0, nullptr, data, 0, 0);
	
	ID3D11DeviceContext* ctx = engine->GetDeferredContextByThread();

	D3D11_MAPPED_SUBRESOURCE res;
	if(XR_SUCCESS == ctx->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res))
	{
		// Copy data
		memcpy(res.pData, data, res.DepthPitch);

		ctx->Unmap(Buffer, 0);

		BufferDirty = true;
	}
}

/** Binds the buffer */
void D3D11ConstantBuffer::BindToVertexShader(int slot)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	engine->GetContext()->VSSetConstantBuffers(slot, 1, &Buffer);

	BufferDirty = false;
}

void D3D11ConstantBuffer::BindToPixelShader(int slot)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	engine->GetContext()->PSSetConstantBuffers(slot, 1, &Buffer);

	BufferDirty = false;
}

void D3D11ConstantBuffer::BindToDomainShader(int slot)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	engine->GetContext()->DSSetConstantBuffers(slot, 1, &Buffer);

	BufferDirty = false;
}

void D3D11ConstantBuffer::BindToHullShader(int slot)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	engine->GetContext()->HSSetConstantBuffers(slot, 1, &Buffer);

	BufferDirty = false;
}

void D3D11ConstantBuffer::BindToGeometryShader(int slot)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	engine->GetContext()->GSSetConstantBuffers(slot, 1, &Buffer);

	BufferDirty = false;
}

/** Returns whether this buffer has been updated since the last bind */
bool D3D11ConstantBuffer::IsDirty()
{
	return BufferDirty;
}