#include "pch.h"
#include "D3D11VertexBuffer.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include <d3dx9mesh.h>

#pragma comment(lib, "d3dx9.lib")

D3D11VertexBuffer::D3D11VertexBuffer(void)
{
	VertexBuffer = NULL;
	ShaderResourceView = NULL;
}


D3D11VertexBuffer::~D3D11VertexBuffer(void)
{
	if(VertexBuffer)VertexBuffer->Release();
	if(ShaderResourceView)ShaderResourceView->Release();
}

/** Creates the vertexbuffer with the given arguments */
XRESULT D3D11VertexBuffer::Init(void* initData, unsigned int sizeInBytes, EBindFlags EBindFlags, EUsageFlags usage, ECPUAccessFlags cpuAccess, const std::string& fileName, unsigned int structuredByteSize)
{
	HRESULT hr;
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	if (sizeInBytes == 0)
	{
		LogError() << "VertexBuffer size can't be 0!";
	}

	SizeInBytes = sizeInBytes;

	// Create our own vertexbuffer
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeInBytes;
	bufferDesc.Usage = (D3D11_USAGE)usage;
	bufferDesc.BindFlags = (D3D11_BIND_FLAG)EBindFlags;
	bufferDesc.CPUAccessFlags = (D3D11_CPU_ACCESS_FLAG)cpuAccess;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = structuredByteSize;

	// Check for structured buffer
	if((EBindFlags & EBindFlags::B_SHADER_RESOURCE) != 0)
		bufferDesc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	// In case we dont have data, allocate some to satisfy D3D11
	char* data = NULL;
	if (!initData)
	{
		data = new char[bufferDesc.ByteWidth];
		memset(data, 0, bufferDesc.ByteWidth);

		initData = data;
	}

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = initData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	LE(engine->GetDevice()->CreateBuffer(&bufferDesc, &InitData, &VertexBuffer));

	// Check for structured buffer again to create the SRV
	if((EBindFlags & EBindFlags::B_SHADER_RESOURCE) != 0)
	{
		 D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory( &srvDesc, sizeof(srvDesc) );
		srvDesc.Format              = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.ElementWidth = sizeInBytes / structuredByteSize;

		engine->GetDevice()->CreateShaderResourceView(VertexBuffer, &srvDesc, &ShaderResourceView);
	}

#ifndef PUBLIC_RELEASE
	VertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, fileName.size(), fileName.c_str());
#endif

	delete[] data;

	return XR_SUCCESS;
}

/** Updates the vertexbuffer with the given data */
XRESULT D3D11VertexBuffer::UpdateBuffer(void* data, UINT size)
{
	void* mappedData;
	UINT bsize;

	if(SizeInBytes < size)
		size = SizeInBytes;

	if (XR_SUCCESS == Map(EMapFlags::M_WRITE_DISCARD, &mappedData, &bsize))
	{
		if (size)
			bsize = size;
		// Copy data
		memcpy(mappedData, data, bsize);

		Unmap();

		return XR_SUCCESS;
	}

	return XR_FAILED;
}

/** Updates the vertexbuffer with the given data */
XRESULT D3D11VertexBuffer::UpdateBufferDeferred(void* data, UINT size)
{
	void* mappedData;
	UINT bsize;

	if(SizeInBytes < size)
		size = SizeInBytes;

	if (XR_SUCCESS == MapDeferred(EMapFlags::M_WRITE_DISCARD, &mappedData, &bsize))
	{
		if (size)
			bsize = size;
		// Copy data
		memcpy(mappedData, data, bsize);

		UnmapDeferred();

		return XR_SUCCESS;
	}

	return XR_FAILED;
}

/** Updates the vertexbuffer with the given data */
XRESULT D3D11VertexBuffer::UpdateBufferAligned16(void* data, UINT size)
{
	void* mappedData;
	UINT bsize;
	if (XR_SUCCESS == Map(EMapFlags::M_WRITE_DISCARD, &mappedData, &bsize))
	{
		if (size)
			bsize = size;
		
		// Copy data
		//Toolbox::X_aligned_memcpy_sse2(mappedData, data, bsize);
		memcpy(mappedData, data, bsize);

		Unmap();

		return XR_SUCCESS;
	}

	return XR_FAILED;
}

/** Maps the buffer */
XRESULT D3D11VertexBuffer::Map(int flags, void** dataPtr, UINT* size)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	D3D11_MAPPED_SUBRESOURCE res;
	if (FAILED(engine->GetContext()->Map(VertexBuffer, 0, (D3D11_MAP)flags, 0, &res)))
		return XR_FAILED;

	*dataPtr = res.pData;
	*size = res.DepthPitch;

	return XR_SUCCESS;
}

/** Unmaps the buffer */
XRESULT D3D11VertexBuffer::Unmap()
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	engine->GetContext()->Unmap(VertexBuffer, 0);

	return XR_SUCCESS;
}

/** Maps the buffer */
XRESULT D3D11VertexBuffer::MapDeferred(int flags, void** dataPtr, UINT* size)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	ID3D11DeviceContext* ctx = engine->GetDeferredContextByThread();

	D3D11_MAPPED_SUBRESOURCE res;
	if (FAILED(ctx->Map(VertexBuffer, 0, (D3D11_MAP)flags, 0, &res)))
		return XR_FAILED;

	*dataPtr = res.pData;
	*size = res.DepthPitch;

	return XR_SUCCESS;
}

/** Unmaps the buffer */
XRESULT D3D11VertexBuffer::UnmapDeferred()
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	ID3D11DeviceContext* ctx = engine->GetDeferredContextByThread();

	ctx->Unmap(VertexBuffer, 0);

	return XR_SUCCESS;
}

/** Returns the D3D11-Buffer object */
ID3D11Buffer* D3D11VertexBuffer::GetVertexBuffer()
{
	return VertexBuffer;
}

/** Optimizes the given set of vertices */
XRESULT D3D11VertexBuffer::OptimizeVertices(VERTEX_INDEX* indices, byte* vertices, unsigned int numIndices, unsigned int numVertices, unsigned int stride)
{
	return XR_SUCCESS;

	DWORD* remap = new DWORD[numVertices];
	if(FAILED(D3DXOptimizeVertices(indices, numIndices / 3, numVertices, FALSE, (DWORD *)remap)))
	{
		delete[] remap;
		return XR_FAILED;
	}

	// Remap vertices
	byte* vxCopy = new byte[numVertices * stride];
	memcpy(vxCopy, vertices, numVertices * stride);

	for(unsigned int i=0;i<numVertices;i++)
	{
		// Assign the vertex at remap[i] to its new vertex
		memcpy(vertices + remap[i] * stride, vxCopy + i * stride, stride);
	}

	for(unsigned int i=0;i<numIndices;i++)
	{
		// Remap the indices.
		indices[i] = (VERTEX_INDEX)remap[indices[i]];
	}

	delete[] vxCopy;
	delete[] remap;

	return XR_SUCCESS;
}

/** Optimizes the given set of vertices */
XRESULT D3D11VertexBuffer::OptimizeFaces(VERTEX_INDEX* indices, byte* vertices, unsigned int numIndices, unsigned int numVertices, unsigned int stride)
{
	return XR_SUCCESS;
	unsigned int numFaces = numIndices / 3;
	DWORD* remap = new DWORD[numFaces];
	if(FAILED(D3DXOptimizeFaces(indices, numFaces, numVertices, FALSE, (DWORD *)remap)))
	{
		delete[] remap;
		return XR_FAILED;
	}
	// Remap vertices
	VERTEX_INDEX* ibCopy = new VERTEX_INDEX[numFaces * 3];
	memcpy(ibCopy, indices, numFaces * 3 * sizeof(VERTEX_INDEX));

	for(unsigned int i=0;i<numFaces;i++)
	{
		// Copy the remapped face
		memcpy( &indices[i * 3], &ibCopy[ remap[i] * 3], 3 * sizeof(VERTEX_INDEX));
	}

	delete[] ibCopy;
	delete[] remap;

	return XR_SUCCESS;
}

/** Returns the size in bytes of this buffer */
unsigned int D3D11VertexBuffer::GetSizeInBytes()
{
	return SizeInBytes;
}

/** Returns the SRV of this buffer, if it represents a structured buffer */
ID3D11ShaderResourceView* D3D11VertexBuffer::GetShaderResourceView()
{
	return ShaderResourceView;
}
