#pragma once
class BaseConstantBuffer
{
public:
	BaseConstantBuffer(int size, void* data);
	virtual ~BaseConstantBuffer(void);

	/** Updates the buffer */
	virtual void UpdateBuffer(void* data) = 0;

	/** Updates the buffer, threadsave */
	virtual void UpdateBufferDeferred(void* data) = 0;

	/** Binds the buffer */
	virtual void BindToVertexShader(int slot) = 0;
	virtual void BindToPixelShader(int slot) = 0;
	virtual void BindToDomainShader(int slot) = 0;
	virtual void BindToHullShader(int slot) = 0;
	virtual void BindToGeometryShader(int slot) = 0;
};

