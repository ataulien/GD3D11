#pragma once
#include "pch.h"

/** Base texture object */
class BaseTexture
{
public:
	BaseTexture(void);
	virtual ~BaseTexture(void);

	/** Layec out for DXGI */
	enum ETextureFormat
	{
		TF_R8G8B8A8 = DXGI_FORMAT_R8G8B8A8_UNORM,
		TF_DXT1 = DXGI_FORMAT_BC1_UNORM,
		TF_DXT3 = DXGI_FORMAT_BC2_UNORM,
		TF_DXT5 = DXGI_FORMAT_BC3_UNORM
	};

	/** Initializes the texture object */
	virtual XRESULT Init(INT2 size, ETextureFormat format, UINT mipMapCount, void* data = NULL, const std::string& fileName = "") = 0;

	/** Initializes the texture from a file */
	virtual XRESULT Init(const std::string& file) = 0;

	/** Updates the Texture-Object */
	virtual XRESULT UpdateData(void* data, int mip) = 0;

	/** Updates the Texture-Object using the deferred context (For loading in an other thread) */
	virtual XRESULT UpdateDataDeferred(void* data, int mip, bool noLock = false) = 0;

	/** Returns the RowPitch-Bytes */
	virtual UINT GetRowPitchBytes(int mip) = 0;

	/** Returns the size of the texture in bytes */
	virtual UINT GetSizeInBytes(int mip) = 0;

	/** Binds this texture to a pixelshader */
	virtual XRESULT BindToPixelShader(int slot) = 0;

	/** Binds this texture to a pixelshader */
	virtual XRESULT BindToVertexShader(int slot) = 0;

	/** Generates mipmaps for this texture (may be slow!) */
	virtual XRESULT GenerateMipMaps() = 0;

	/** Returns this textures ID */
	UINT16 GetID(){return ID;};

	

protected:
	/** The ID of this texture */
	UINT16 ID;
};

