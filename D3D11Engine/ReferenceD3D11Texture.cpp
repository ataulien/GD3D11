#include "pch.h"
#include "ReferenceD3D11Texture.h"
#include "Engine.h"
#include "ReferenceD3D11GraphicsEngine.h"
#include "GothicAPI.h"

ReferenceD3D11Texture::ReferenceD3D11Texture(void)
{
	Texture = NULL;
	ShaderResourceView = NULL;
}


ReferenceD3D11Texture::~ReferenceD3D11Texture(void)
{
	if(Texture)Texture->Release();
	if(ShaderResourceView)ShaderResourceView->Release();
}

/** Initializes the texture object */
XRESULT ReferenceD3D11Texture::Init(INT2 size, ETextureFormat format, UINT mipMapCount, void* data)
{
	HRESULT hr;
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	TextureFormat = (DXGI_FORMAT)format;
	TextureSize = size;

	CD3D11_TEXTURE2D_DESC textureDesc(
		(DXGI_FORMAT)format,
		size.x,
		size.y,
		1,
		mipMapCount,
		D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, 1, 0, 0);

	LE( engine->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &Texture) ); 

	D3D11_SHADER_RESOURCE_VIEW_DESC descRV;
	ZeroMemory(&descRV, sizeof(descRV));
	descRV.Format = DXGI_FORMAT_UNKNOWN;
	descRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descRV.Texture2D.MipLevels = mipMapCount;
	descRV.Texture2D.MostDetailedMip = 0;
	LE( engine->GetDevice()->CreateShaderResourceView(Texture, &descRV, &ShaderResourceView) );

	return XR_SUCCESS;
}

/** Updates the Texture-Object */
XRESULT ReferenceD3D11Texture::UpdateData(void* data, int mip)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	// Enter the critical section for safety while executing the deferred command list
	Engine::GAPI->EnterResourceCriticalSection();
	engine->GetContext()->UpdateSubresource(Texture, mip, NULL, data, GetRowPitchBytes(mip), GetSizeInBytes(mip));
	Engine::GAPI->LeaveResourceCriticalSection();

	return XR_SUCCESS;
}

/** Updates the Texture-Object using the deferred context (For loading in an other thread) */
XRESULT ReferenceD3D11Texture::UpdateDataDeferred(void* data, int mip)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	// Enter the critical section for safety while executing the deferred command list
	Engine::GAPI->EnterResourceCriticalSection();
	engine->GetDeferredContext()->UpdateSubresource(Texture, mip, NULL, data, GetRowPitchBytes(mip), GetSizeInBytes(mip));
	Engine::GAPI->LeaveResourceCriticalSection();

	return XR_SUCCESS;
}

/** Returns the RowPitch-Bytes */
UINT ReferenceD3D11Texture::GetRowPitchBytes(int mip)
{
	int px = std::max(1.0f, floor(TextureSize.x / pow(2.0f, mip)));
	int py = std::max(1.0f, floor(TextureSize.y / pow(2.0f, mip)));
	//int px = TextureSize.x;
	//int py = TextureSize.y;

	if(TextureFormat == DXGI_FORMAT_BC1_UNORM || TextureFormat == DXGI_FORMAT_BC2_UNORM ||
		TextureFormat == DXGI_FORMAT_BC3_UNORM)
	{
		return Toolbox::GetDDSRowPitchSize(px, TextureFormat == DXGI_FORMAT_BC1_UNORM);
	}else // Use R8G8B8A8
	{
		return px * 4;
	}
}

/** Returns the size of the texture in bytes */
UINT ReferenceD3D11Texture::GetSizeInBytes(int mip)
{
	int px = std::max(1.0f, floor(TextureSize.x / pow(2.0f, mip)));
	int py = std::max(1.0f, floor(TextureSize.y / pow(2.0f, mip)));
	//int px = TextureSize.x;
	//int py = TextureSize.y;

	if(TextureFormat == DXGI_FORMAT_BC1_UNORM || TextureFormat == DXGI_FORMAT_BC2_UNORM ||
		TextureFormat == DXGI_FORMAT_BC3_UNORM)
	{
		return Toolbox::GetDDSStorageRequirements(px, py, TextureFormat == DXGI_FORMAT_BC1_UNORM);
	}else // Use R8G8B8A8
	{
		return px * py * 4;
	}
}

/** Binds this texture to a pixelshader */
XRESULT ReferenceD3D11Texture::BindToPixelShader(int slot)
{
	ReferenceD3D11GraphicsEngine* engine = (ReferenceD3D11GraphicsEngine *)Engine::GraphicsEngine;

	engine->GetContext()->PSSetShaderResources(slot, 1, &ShaderResourceView);

	return XR_SUCCESS;
}
