#include "pch.h"
#include "ReferenceD3D11ConstantBuffer.h"
#include "ReferenceD3D11GraphicsEngine.h"
#include "Engine.h"
#include "GothicAPI.h"

ReferenceD3D11ConstantBuffer::ReferenceD3D11ConstantBuffer(int size, void* data)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	char* dd = new char[size];
	ZeroMemory(dd, size);

	D3D11_SUBRESOURCE_DATA d;
	d.pSysMem = dd;
	d.SysMemPitch=0;
	d.SysMemSlicePitch=0;

	
	// Create constantbuffer
	HRESULT hr;
	LE( engine->GetDevice()->CreateBuffer(	&CD3D11_BUFFER_DESC(size, D3D11_BIND_CONSTANT_BUFFER),
															&d,
															&Buffer) );

	delete[] dd;
}


ReferenceD3D11ConstantBuffer::~ReferenceD3D11ConstantBuffer(void)
{
	if(Buffer)Buffer->Release();
}

/** Updates the buffer */
void ReferenceD3D11ConstantBuffer::UpdateBuffer(void* data)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->UpdateSubresource(Buffer,  0, nullptr, data, 0, 0);
}

/** Binds the buffer */
void ReferenceD3D11ConstantBuffer::BindToVertexShader(int slot)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->VSSetConstantBuffers(slot, 1, &Buffer);
}

void ReferenceD3D11ConstantBuffer::BindToPixelShader(int slot)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->PSSetConstantBuffers(slot, 1, &Buffer);
}

void ReferenceD3D11ConstantBuffer::BindToDomainShader(int slot)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;
	engine->GetContext()->DSSetConstantBuffers(slot, 1, &Buffer);
}
