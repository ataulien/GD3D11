#pragma once
#include "basetexture.h"
class ReferenceD3D11Texture :
	public BaseTexture
{
public:
	ReferenceD3D11Texture(void);
	~ReferenceD3D11Texture(void);

	/** Initializes the texture object */
	virtual XRESULT Init(INT2 size, ETextureFormat format, UINT mipMapCount = 1, void* data = NULL);

	/** Initializes the texture from a file */
	virtual XRESULT Init(const std::string& file){return XR_SUCCESS;};

	/** Updates the Texture-Object */
	virtual XRESULT UpdateData(void* data, int mip=0);

	/** Updates the Texture-Object using the deferred context (For loading in an other thread) */
	virtual XRESULT UpdateDataDeferred(void* data, int mip);

	/** Returns the RowPitch-Bytes */
	virtual UINT GetRowPitchBytes(int mip);

	/** Returns the size of the texture in bytes */
	virtual UINT GetSizeInBytes(int mip);

	/** Binds this texture to a pixelshader */
	virtual XRESULT BindToPixelShader(int slot);
private:
	/** D3D11 objects */
	ID3D11Texture2D* Texture;
	ID3D11ShaderResourceView* ShaderResourceView;
	DXGI_FORMAT TextureFormat;
	INT2 TextureSize;
};

