#pragma once
#include "basetexture.h"
class D3D11Texture :
	public BaseTexture
{
public:
	D3D11Texture(void);
	~D3D11Texture(void);

	/** Initializes the texture object */
	virtual XRESULT Init(INT2 size, ETextureFormat format, UINT mipMapCount = 1, void* data = NULL, const std::string& fileName = "");

	/** Initializes the texture from a file */
	virtual XRESULT Init(const std::string& file);

	/** Updates the Texture-Object */
	virtual XRESULT UpdateData(void* data, int mip = 0);

	/** Updates the Texture-Object using the deferred context (For loading in an other thread) */
	virtual XRESULT UpdateDataDeferred(void* data, int mip, bool noLock = false);

	/** Returns the RowPitch-Bytes */
	virtual UINT GetRowPitchBytes(int mip);

	/** Returns the size of the texture in bytes */
	virtual UINT GetSizeInBytes(int mip);

	/** Binds this texture to a pixelshader */
	virtual XRESULT BindToPixelShader(int slot);

	/** Binds this texture to a pixelshader */
	virtual XRESULT BindToVertexShader(int slot);

	/** Binds this texture to a domainshader */
	virtual XRESULT BindToDomainShader(int slot);

	/** Returns the texture-object */
	ID3D11Texture2D* GetTextureObject(){return Texture;}

	/** Returns the shader resource view */
	ID3D11ShaderResourceView* GetShaderResourceView(){return ShaderResourceView;}

	/** Creates a thumbnail for this */
	XRESULT CreateThumbnail();

	/** Returns the thumbnail of this texture. If this returns NULL, you need to create one first */
	ID3D11Texture2D* GetThumbnail();

	/** Generates mipmaps for this texture (may be slow!) */
	XRESULT GenerateMipMaps();

private:
	/** D3D11 objects */
	ID3D11Texture2D* Texture;
	ID3D11ShaderResourceView* ShaderResourceView;
	DXGI_FORMAT TextureFormat;
	INT2 TextureSize;
	int MipMapCount;

	/** Thumbnail */
	ID3D11Texture2D* Thumbnail;
};

