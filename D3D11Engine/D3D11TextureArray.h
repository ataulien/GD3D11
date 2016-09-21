#pragma once
class D3D11TextureArray
{
public:
	D3D11TextureArray(void);
	~D3D11TextureArray(void);

	/** Adds a texture with data to the array, returns the index of texture */
	int AddTexture(byte* data, unsigned int dataSize);

	/** Creates the texture array and deletes the cached data */
	void InitArray();

private:

	/** Clears the cache */
	void ClearCache();

	/** Cache array */
	std::vector<byte*> DataCache;
};

