#include "pch.h"
#include "ReferenceD3D11VertexBuffer.h"
#include "ReferenceD3D11GraphicsEngine.h"
#include "Engine.h"

ReferenceD3D11VertexBuffer::ReferenceD3D11VertexBuffer(void)
{
	VertexBuffer = NULL;
}


ReferenceD3D11VertexBuffer::~ReferenceD3D11VertexBuffer(void)
{
	if(VertexBuffer)VertexBuffer->Release();
}

/** Creates the vertexbuffer with the given arguments */
XRESULT ReferenceD3D11VertexBuffer::Init(void* initData, unsigned int sizeInBytes, EBindFlags EBindFlags, EUsageFlags usage, ECPUAccessFlags cpuAccess)
{
	HRESULT hr;
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	if(sizeInBytes == 0)
	{
		LogError() << "VertexBuffer size can't be 0!";
	}

	// Create our own vertexbuffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeInBytes;
	bufferDesc.Usage = (D3D11_USAGE)usage;
	bufferDesc.BindFlags = (D3D11_BIND_FLAG)EBindFlags;
	bufferDesc.CPUAccessFlags = (D3D11_CPU_ACCESS_FLAG)cpuAccess;
	bufferDesc.MiscFlags = 0;

	// In case we dont have data, allocate some to statisfy D3D11
	char* data = NULL;
	if(!initData)
	{
		data = new char[bufferDesc.ByteWidth];
		memset(data, 0, bufferDesc.ByteWidth);
		initData = data;
	}

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = initData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	LE(engine->GetDevice()->CreateBuffer(&bufferDesc,&InitData,&VertexBuffer));

	delete[] data;

	return XR_SUCCESS;
}


/** Updates the vertexbuffer with the given data */
XRESULT ReferenceD3D11VertexBuffer::UpdateBuffer(void* data, UINT size)
{
	void* mappedData;
	UINT bsize;
	if(XR_SUCCESS == Map(EMapFlags::M_WRITE_DISCARD, &mappedData, &bsize))
	{
		if(size)
			bsize = size;
		// Copy data
		memcpy(mappedData, data, bsize);

		Unmap();
	}

	return XR_SUCCESS;
}


/** Maps the buffer */
XRESULT ReferenceD3D11VertexBuffer::Map(int flags, void** dataPtr, UINT* size)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	D3D11_MAPPED_SUBRESOURCE res;
	if(FAILED(engine->GetContext()->Map(VertexBuffer, 0, (D3D11_MAP)flags, 0, &res)))
		return XR_FAILED;

	*dataPtr = res.pData;
	*size = res.DepthPitch;

	return XR_SUCCESS;
}

/** Unmaps the buffer */
XRESULT ReferenceD3D11VertexBuffer::Unmap()
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	engine->GetContext()->Unmap(VertexBuffer, 0);

	return XR_SUCCESS;
}

/** Returns the D3D11-Buffer object */
ID3D11Buffer* ReferenceD3D11VertexBuffer::GetVertexBuffer()
{
	return VertexBuffer;
}

/** Create triangle */
void ReferenceD3D11VertexBuffer::CreateTriangle()
{
	ExVertexStruct vx[3];
	ZeroMemory(vx, sizeof(vx));

	float scale = 50.0f;
	vx[0].Position = float3(0.0f, 0.5f * scale, 0.0f);
	vx[1].Position = float3(0.45f * scale, -0.5f * scale, 0.0f);
	vx[2].Position = float3(-0.45f * scale, -0.5f * scale, 0.0f);

	vx[0].Color = float4(1,0,0,1).ToDWORD();
	vx[1].Color = float4(0,1,0,1).ToDWORD();
	vx[2].Color = float4(0,0,1,1).ToDWORD();

	UpdateBuffer(vx);
}