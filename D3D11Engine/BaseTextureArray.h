#pragma once
class BaseTextureArray
{
public:
	BaseTextureArray(void);
	virtual ~BaseTextureArray(void);

	/** Adds a texture with data to the array, returns the index of texture */
	virtual int AddTexture(byte* data, unsigned int dataSize) = 0;

	/** Creates the texture array and deletes the cached data */
	virtual void InitArray() = 0;
};

