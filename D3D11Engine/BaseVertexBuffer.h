#pragma once
#include "pch.h"

/** Base vertex buffer object */
class BaseVertexBuffer
{
public:
	BaseVertexBuffer(void);
	virtual ~BaseVertexBuffer(void);

	/** Layed out for D3D11*/
	enum ECPUAccessFlags
	{
		CA_NONE = 0,
		CA_WRITE = 0x10000L,
		CA_READ = 0x20000L,
	};

	/** Layed out for D3D11*/
	enum EUsageFlags
	{
		U_DEFAULT = 0,
		U_DYNAMIC = 2,
		U_IMMUTABLE = 1
	};

	/** Layed out for D3D11*/
	enum EMapFlags
	{
		M_READ = 1,
		M_WRITE = 2,
		M_READ_WRITE = 3,
		M_WRITE_DISCARD = 4,
	};

	/** Layed out for D3D11*/
	enum EBindFlags
	{
		B_VERTEXBUFFER = D3D11_BIND_VERTEX_BUFFER,
		B_INDEXBUFFER = D3D11_BIND_INDEX_BUFFER,
	};

	/** Creates the vertexbuffer with the given arguments */
	virtual XRESULT Init(void* initData, unsigned int sizeInBytes, EBindFlags EBindFlags = B_VERTEXBUFFER, EUsageFlags usage = EUsageFlags::U_DEFAULT, ECPUAccessFlags cpuAccess = ECPUAccessFlags::CA_NONE, const std::string& fileName = "") = 0;

	/** Updates the vertexbuffer with the given data */
	virtual XRESULT UpdateBuffer(void* data, UINT size = 0) = 0;

	/** Updates the vertexbuffer with the given data */
	virtual XRESULT UpdateBufferAligned16(void* data, UINT size = 0) = 0;

	/** Maps the buffer */
	virtual XRESULT Map(int flags, void** dataPtr, UINT* size) = 0;

	/** Unmaps the buffer */
	virtual XRESULT Unmap() = 0;

	/** Optimizes the given set of vertices */
	virtual XRESULT OptimizeVertices(VERTEX_INDEX* indices, byte* vertices, unsigned int numIndices, unsigned int numVertices, unsigned int stride) = 0;

	/** Optimizes the given set of vertices */
	virtual XRESULT OptimizeFaces(VERTEX_INDEX* indices, byte* vertices, unsigned int numIndices, unsigned int numVertices, unsigned int stride) = 0;

	/** Returns the size in bytes of this buffer */
	virtual unsigned int GetSizeInBytes() = 0;
};

