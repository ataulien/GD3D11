#include "pch.h"
#include "D3D11Texture.h"
#include "Engine.h"
#include "D3D11GraphicsEngineBase.h"
#include "GothicAPI.h"
#include <D3DX11.h>
#include "RenderToTextureBuffer.h"

D3D11Texture::D3D11Texture(void)
{
	Texture = nullptr;
	ShaderResourceView = nullptr;
	Thumbnail = nullptr;

	// Insert into state-map
	ID = D3D11ObjectIDs::Counters.TextureCounter++;

	D3D11ObjectIDs::TextureByID[ID] = this;
}

D3D11Texture::~D3D11Texture(void)
{
	// Remove from state map
	Toolbox::EraseByElement(D3D11ObjectIDs::TextureByID, this);

	if (Thumbnail)Thumbnail->Release();
	if (Texture)Texture->Release();
	if (ShaderResourceView)ShaderResourceView->Release();
}

/** Initializes the texture object */
XRESULT D3D11Texture::Init(INT2 size, ETextureFormat format, UINT mipMapCount, void* data, const std::string& fileName)
{
	HRESULT hr;
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	//Engine::GAPI->EnterResourceCriticalSection();

	TextureFormat = (DXGI_FORMAT)format;
	TextureSize = size;
	MipMapCount = mipMapCount;

	CD3D11_TEXTURE2D_DESC textureDesc(
		(DXGI_FORMAT)format,
		size.x,
		size.y,
		1,
		mipMapCount,
		D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, 1, 0, 0);

	LE(engine->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &Texture));

#ifndef PUBLIC_RELEASE
	Texture->SetPrivateData(WKPDID_D3DDebugObjectName, fileName.size(), fileName.c_str());
#endif

	D3D11_SHADER_RESOURCE_VIEW_DESC descRV;
	ZeroMemory(&descRV, sizeof(descRV));
	descRV.Format = DXGI_FORMAT_UNKNOWN;
	descRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descRV.Texture2D.MipLevels = mipMapCount;
	descRV.Texture2D.MostDetailedMip = 0;
	LE(engine->GetDevice()->CreateShaderResourceView(Texture, &descRV, &ShaderResourceView));

	//Engine::GAPI->LeaveResourceCriticalSection();

	return XR_SUCCESS;
}

/** Initializes the texture from a file */
XRESULT D3D11Texture::Init(const std::string& file)
{
	HRESULT hr;
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	//LogInfo() << "Loading Engine-Texture: " << file;

	//Engine::GAPI->EnterResourceCriticalSection();

	LE(D3DX11CreateShaderResourceViewFromFileA(engine->GetDevice(), file.c_str(), NULL, NULL, &ShaderResourceView, NULL));

	if(!ShaderResourceView)
		return XR_FAILED;
	
	ID3D11Texture2D* res;
	ShaderResourceView->GetResource((ID3D11Resource **)&res);

	D3D11_TEXTURE2D_DESC desc;
	res->GetDesc(&desc);

	Texture = res;
	TextureFormat = desc.Format;

	TextureSize.x = desc.Width;
	TextureSize.y = desc.Height;

	//Engine::GAPI->LeaveResourceCriticalSection();

	return XR_SUCCESS;
}

/** Updates the Texture-Object */
XRESULT D3D11Texture::UpdateData(void* data, int mip)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	// Enter the critical section for safety while executing the deferred command list
	Engine::GAPI->EnterResourceCriticalSection();
	engine->GetContext()->UpdateSubresource(Texture, mip, nullptr, data, GetRowPitchBytes(mip), GetSizeInBytes(mip));
	Engine::GAPI->LeaveResourceCriticalSection();

	return XR_SUCCESS;
}

/** Updates the Texture-Object using the deferred context (For loading in an other thread) */
XRESULT D3D11Texture::UpdateDataDeferred(void* data, int mip, bool noLock)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	// Enter the critical section for safety while executing the deferred command list
	if(!noLock)
		Engine::GAPI->EnterResourceCriticalSection();

	engine->GetDeferredMediaContext()->UpdateSubresource(Texture, mip, nullptr, data, GetRowPitchBytes(mip), GetSizeInBytes(mip));

	if(!noLock)
		Engine::GAPI->LeaveResourceCriticalSection();

	return XR_SUCCESS;
}

/** Returns the RowPitch-Bytes */
UINT D3D11Texture::GetRowPitchBytes(int mip) const
{
	int px = (int)std::max(1.0f, floor(TextureSize.x / pow(2.0f, mip)));
	int py = (int)std::max(1.0f, floor(TextureSize.y / pow(2.0f, mip)));
	//int px = TextureSize.x;
	//int py = TextureSize.y;

	if (TextureFormat == DXGI_FORMAT_BC1_UNORM || TextureFormat == DXGI_FORMAT_BC2_UNORM ||
		TextureFormat == DXGI_FORMAT_BC3_UNORM)
	{
		return Toolbox::GetDDSRowPitchSize(px, TextureFormat == DXGI_FORMAT_BC1_UNORM);
	}
	else // Use R8G8B8A8
	{
		return px * 4;
	}
}

/** Returns the size of the texture in bytes */
UINT D3D11Texture::GetSizeInBytes(int mip) const
{
	int px = (int)std::max(1.0f, floor(TextureSize.x / pow(2.0f, mip)));
	int py = (int)std::max(1.0f, floor(TextureSize.y / pow(2.0f, mip)));
	//int px = TextureSize.x;
	//int py = TextureSize.y;

	if (TextureFormat == DXGI_FORMAT_BC1_UNORM || TextureFormat == DXGI_FORMAT_BC2_UNORM ||
		TextureFormat == DXGI_FORMAT_BC3_UNORM)
	{
		return Toolbox::GetDDSStorageRequirements(px, py, TextureFormat == DXGI_FORMAT_BC1_UNORM);
	}
	else // Use R8G8B8A8
	{
		return px * py * 4;
	}
}

/** Binds this texture to a pixelshader */
XRESULT D3D11Texture::BindToPixelShader(int slot) const
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	ID3D11ShaderResourceView* srv = ShaderResourceView;
	engine->GetContext()->PSSetShaderResources(slot, 1, &srv);

	return XR_SUCCESS;
}

/** Binds this texture to a pixelshader */
XRESULT D3D11Texture::BindToVertexShader(int slot) const
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	ID3D11ShaderResourceView* srv = ShaderResourceView;
	engine->GetContext()->VSSetShaderResources(slot, 1, &srv);
	engine->GetContext()->DSSetShaderResources(slot, 1, &srv);

	return XR_SUCCESS;
}

/** Binds this texture to a domainshader */
XRESULT D3D11Texture::BindToDomainShader(int slot) const
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	ID3D11ShaderResourceView* srv = ShaderResourceView;
	engine->GetContext()->DSSetShaderResources(slot, 1, &srv);

	return XR_SUCCESS;
}

/** Creates a thumbnail for this */
XRESULT D3D11Texture::CreateThumbnail()
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	HRESULT hr;
	// Since D2D can't load DXTn-Textures on Windows 7, we copy it over to a smaller texture here in d3d11

	// Create texture
	CD3D11_TEXTURE2D_DESC textureDesc(
		(DXGI_FORMAT)DXGI_FORMAT_R8G8B8A8_UNORM,
		256,
		256,
		1,
		1,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, 1, 0, 0);

	LE(engine->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &Thumbnail));

	// Create temporary RTV
	ID3D11RenderTargetView* tempRTV;
	LE(engine->GetDevice()->CreateRenderTargetView(Thumbnail, NULL, &tempRTV));
	if(!tempRTV)
		return XR_FAILED;

	engine->GetContext()->ClearRenderTargetView(tempRTV, (float *)&D3DXVECTOR4(1,0,0,1));

	// Copy main texture to it
	engine->GetContext()->PSSetShaderResources(0, 1, &ShaderResourceView);

	ID3D11RenderTargetView* oldRTV[2];
	ID3D11DepthStencilView* oldDSV;

	engine->GetContext()->OMGetRenderTargets(2, oldRTV, &oldDSV);

	engine->GetContext()->OMSetRenderTargets(1, &tempRTV, nullptr);
	engine->DrawQuad(INT2(0,0), INT2(256,256));

	engine->GetContext()->OMSetRenderTargets(2, oldRTV, oldDSV);

	if(oldRTV[0])oldRTV[0]->Release();
	if(oldRTV[1])oldRTV[1]->Release();
	if(oldDSV)oldDSV->Release();

	tempRTV->Release();
	return XR_SUCCESS;
}

/** Returns the thumbnail of this texture. If this returns NULL, you need to create one first */
ID3D11Texture2D* D3D11Texture::GetThumbnail() const
{
	return Thumbnail;
}

/** Generates mipmaps for this texture (may be slow!) */
XRESULT D3D11Texture::GenerateMipMaps() const
{
	if(MipMapCount == 1)
		return XR_SUCCESS;

	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	Engine::GAPI->EnterResourceCriticalSection();

	RenderToTextureBuffer* b = new RenderToTextureBuffer(engine->GetDevice(), TextureSize.x, TextureSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, nullptr, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, MipMapCount);

	engine->GetDeferredMediaContext()->CopySubresourceRegion(b->GetTexture(), 0, 0, 0, 0, Texture, 0, nullptr);

	// Generate mips
	engine->GetDeferredMediaContext()->GenerateMips(b->GetShaderResView());

	// Copy the full chain back
	engine->GetDeferredMediaContext()->CopyResource(Texture, b->GetTexture());
	delete b;

	Engine::GAPI->LeaveResourceCriticalSection();

	return XR_SUCCESS;
}
