#pragma once
#include "BaseConstantBuffer.h"

class D3D11ConstantBuffer : public BaseConstantBuffer
{
public:
	D3D11ConstantBuffer(int size, void* data);
	~D3D11ConstantBuffer(void);

	/** Updates the buffer */
	virtual void UpdateBuffer(void* data);

	/** Binds the buffer */
	void BindToVertexShader(int slot);
	void BindToPixelShader(int slot);
	void BindToDomainShader(int slot);
	void BindToHullShader(int slot);

	/** Binds the constantbuffer */
	ID3D11Buffer* Get(){ return Buffer; }

	/** Returns whether this buffer has been updated since the last bind */
	bool IsDirty();

private:
	ID3D11Buffer* Buffer;
	int OriginalSize; // Buffersize must be a multiple of 16
	bool BufferDirty;
};

