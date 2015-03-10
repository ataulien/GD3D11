#include "pch.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11GraphicsEngine.h"
#include "Engine.h"
#include "GothicAPI.h"

D3D11ConstantBuffer::D3D11ConstantBuffer(int size, void* data) : BaseConstantBuffer(size, data)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

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
	LE(engine->GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER), &d, &Buffer));

	if(!data)
		delete[] dd;
}


D3D11ConstantBuffer::~D3D11ConstantBuffer(void)
{
	if (Buffer)Buffer->Release();
}

/** Updates the buffer */
void D3D11ConstantBuffer::UpdateBuffer(void* data)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->UpdateSubresource(Buffer, 0, nullptr, data, 0, 0);
}

/** Binds the buffer */
void D3D11ConstantBuffer::BindToVertexShader(int slot)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->VSSetConstantBuffers(slot, 1, &Buffer);
}

void D3D11ConstantBuffer::BindToPixelShader(int slot)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->PSSetConstantBuffers(slot, 1, &Buffer);
}

void D3D11ConstantBuffer::BindToDomainShader(int slot)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->DSSetConstantBuffers(slot, 1, &Buffer);
}

void D3D11ConstantBuffer::BindToHullShader(int slot)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->HSSetConstantBuffers(slot, 1, &Buffer);
}