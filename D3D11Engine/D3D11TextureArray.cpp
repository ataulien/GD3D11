#include "pch.h"
#include "D3D11TextureArray.h"


D3D11TextureArray::D3D11TextureArray(void)
{
}


D3D11TextureArray::~D3D11TextureArray(void)
{
}

/** Adds a texture with data to the array, returns the index of texture */
int D3D11TextureArray::AddTexture(byte* data, unsigned int dataSize)
{
	byte* cpy = new byte[dataSize];
	memcpy(cpy, data, dataSize);

	DataCache.push_back(cpy);

	return DataCache.size()-1;
}

/** Clears the cache */
void D3D11TextureArray::ClearCache()
{
}

/** Creates the texture array and deletes the cached data */
void D3D11TextureArray::InitArray()
{

}