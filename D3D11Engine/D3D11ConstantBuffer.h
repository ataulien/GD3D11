#pragma once

class D3D11ConstantBuffer
{
public:
	D3D11ConstantBuffer(int size, void* data);
	~D3D11ConstantBuffer(void);

	/** Updates the buffer */
	void UpdateBuffer(void* data);

	/** Updates the buffer, threadsave */
	void UpdateBufferDeferred(void* data);

	/** Binds the buffer */
	void BindToVertexShader(int slot);
	void BindToPixelShader(int slot);
	void BindToDomainShader(int slot);
	void BindToHullShader(int slot);
	void BindToGeometryShader(int slot);
	
	/** Binds the constantbuffer */
	ID3D11Buffer* Get(){ return Buffer; }

	/** Returns whether this buffer has been updated since the last bind */
	bool IsDirty();

private:
	ID3D11Buffer* Buffer;
	int OriginalSize; // Buffersize must be a multiple of 16
	bool BufferDirty;
};

