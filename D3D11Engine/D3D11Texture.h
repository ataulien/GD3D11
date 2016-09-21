#pragma once
class D3D11Texture
{
public:
	D3D11Texture(void);
	~D3D11Texture(void);

	/** Layec out for DXGI */
	enum ETextureFormat
	{
		TF_R8G8B8A8 = DXGI_FORMAT_R8G8B8A8_UNORM,
		TF_DXT1 = DXGI_FORMAT_BC1_UNORM,
		TF_DXT3 = DXGI_FORMAT_BC2_UNORM,
		TF_DXT5 = DXGI_FORMAT_BC3_UNORM
	};

	/** Initializes the texture object */
	XRESULT Init(INT2 size, ETextureFormat format, UINT mipMapCount = 1, void* data = NULL, const std::string& fileName = "");

	/** Initializes the texture from a file */
	XRESULT Init(const std::string& file);

	/** Updates the Texture-Object */
	XRESULT UpdateData(void* data, int mip = 0);

	/** Updates the Texture-Object using the deferred context (For loading in an other thread) */
	XRESULT UpdateDataDeferred(void* data, int mip, bool noLock = false);

	/** Returns the RowPitch-Bytes */
	UINT GetRowPitchBytes(int mip);

	/** Returns the size of the texture in bytes */
	UINT GetSizeInBytes(int mip);

	/** Binds this texture to a pixelshader */
	XRESULT BindToPixelShader(int slot);

	/** Binds this texture to a pixelshader */
	XRESULT BindToVertexShader(int slot);

	/** Binds this texture to a domainshader */
	XRESULT BindToDomainShader(int slot);

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

	/** Returns this textures ID */
	UINT16 GetID() { return ID; };

private:
	/** The ID of this texture */
	UINT16 ID;

	/** D3D11 objects */
	ID3D11Texture2D* Texture;
	ID3D11ShaderResourceView* ShaderResourceView;
	DXGI_FORMAT TextureFormat;
	INT2 TextureSize;
	int MipMapCount;

	/** Thumbnail */
	ID3D11Texture2D* Thumbnail;
};

