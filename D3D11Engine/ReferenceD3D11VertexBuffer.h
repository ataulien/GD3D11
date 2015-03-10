#pragma once
#include "basevertexbuffer.h"
class ReferenceD3D11VertexBuffer :
	public BaseVertexBuffer
{
public:
	ReferenceD3D11VertexBuffer(void);
	~ReferenceD3D11VertexBuffer(void);

	/** Creates the vertexbuffer with the given arguments */
	virtual XRESULT Init(void* initData, unsigned int sizeInBytes, EBindFlags EBindFlags = B_VERTEXBUFFER, EUsageFlags usage = EUsageFlags::U_DEFAULT, ECPUAccessFlags cpuAccess = ECPUAccessFlags::CA_NONE);

	/** Updates the vertexbuffer with the given data */
	virtual XRESULT UpdateBuffer(void* data, UINT size = 0);

	/** Maps the buffer */
	virtual XRESULT Map(int flags, void** dataPtr, UINT* size);

	/** Unmaps the buffer */
	virtual XRESULT Unmap();

	/** Create triangle */
	void CreateTriangle();

	/** Returns the D3D11-Buffer object */
	ID3D11Buffer* GetVertexBuffer();
private:
	/** Vertex buffer object */
	ID3D11Buffer* VertexBuffer;
};

