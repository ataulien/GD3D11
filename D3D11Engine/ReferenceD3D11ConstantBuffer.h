#pragma once
class ReferenceD3D11ConstantBuffer
{
public:
	ReferenceD3D11ConstantBuffer(int size, void* data);
	~ReferenceD3D11ConstantBuffer(void);

	/** Updates the buffer */
	void UpdateBuffer(void* data);

	/** Binds the buffer */
	void BindToVertexShader(int slot);
	void BindToPixelShader(int slot);
	void BindToDomainShader(int slot);

	/** Binds the constantbuffer */
	ID3D11Buffer* Get(){return Buffer;}

private:
	ID3D11Buffer* Buffer;
	int OriginalSize; // Buffersize must be a multiple of 16
};

