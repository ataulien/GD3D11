#pragma once
#include "pch.h"
#include "Logger.h"
#include <d3d11.h>

/** Helper structs for quickly creating render-to-texture buffers */

/** Struct for a texture that can be used as shader resource AND rendertarget */
struct RenderToTextureBuffer
{
	~RenderToTextureBuffer()
	{
		ReleaseAll();
	}

	/** Creates the render-to-texture buffers */
	RenderToTextureBuffer(ID3D11Device* device, UINT SizeX,UINT SizeY,DXGI_FORMAT Format, HRESULT* Result=NULL, DXGI_FORMAT RTVFormat=DXGI_FORMAT_UNKNOWN, DXGI_FORMAT SRVFormat=DXGI_FORMAT_UNKNOWN, int MipLevels = 1)
	{
		HRESULT hr=S_OK;

		Texture=NULL;
		ShaderResView=NULL;
		RenderTargetView=NULL;

		if(SizeX == 0 || SizeY == 0)
		{
			LogError() << L"SizeX or SizeY can't be 0";
		}

		this->SizeX=SizeX;
		this->SizeY=SizeY;

		if(Format==0)
		{
			LogError() << L"DXGI_FORMAT_UNKNOWN (0) isn't a valid texture format";
		}

		//Create a new render target texture
		D3D11_TEXTURE2D_DESC Desc;
		ZeroMemory( &Desc, sizeof( D3D10_TEXTURE2D_DESC ) );
		Desc.ArraySize = 1;
		Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.Format = Format;
		Desc.Width = SizeX;
		Desc.Height = SizeY;
		Desc.MipLevels = MipLevels;
		Desc.SampleDesc.Count = 1;
		
		if(MipLevels != 1)
			Desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		LE(device->CreateTexture2D(&Desc,NULL,&Texture));

		//Create a render target view
		D3D11_RENDER_TARGET_VIEW_DESC DescRT;
		DescRT.Format = (RTVFormat != DXGI_FORMAT_UNKNOWN ? RTVFormat : Desc.Format);
		DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		DescRT.Texture2D.MipSlice = 0;
		LE(device->CreateRenderTargetView((ID3D11Resource *)Texture,&DescRT,&RenderTargetView));


		// Create the resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC DescRV;
		DescRV.Format = (SRVFormat != DXGI_FORMAT_UNKNOWN ? SRVFormat : Desc.Format);
		DescRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		DescRV.Texture2D.MipLevels = MipLevels;
		DescRV.Texture2D.MostDetailedMip = 0;
		LE(device->CreateShaderResourceView( (ID3D11Resource *)Texture, &DescRV, &ShaderResView ));

		if(FAILED(hr))
		{
			LogError() << L"Coould not create ID3D11Texture2D, ID3D11ShaderResourceView, or ID3D11RenderTargetView. Killing created resources (If any).";
			ReleaseAll();
			if(Result)*Result=hr;
			return;
		}

		//LogInfo() << L"Successfully created ID3D11Texture2D, ID3D11ShaderResourceView, and ID3D11RenderTargetView.";
		if(Result)*Result=hr;
	}

	/** Binds the texture to the pixel shader */
	void BindToPixelShader(ID3D11DeviceContext* context, int slot)
	{
		context->PSSetShaderResources(slot, 1, &ShaderResView);
	};

	ID3D11Texture2D* GetTexture(){return Texture;}
	ID3D11ShaderResourceView* GetShaderResView(){return ShaderResView;}
	ID3D11ShaderResourceView** GetShaderResViewPtr(){return &ShaderResView;}
	ID3D11RenderTargetView* GetRenderTargetView(){return RenderTargetView;}
	ID3D11RenderTargetView** GetRenderTargetViewPtr(){return &RenderTargetView;}



	UINT GetSizeX(){return SizeX;}
	UINT GetSizeY(){return SizeY;}
private:

	/** The Texture object */
	ID3D11Texture2D* Texture;

	UINT SizeX;
	UINT SizeY;

	/** Shader and rendertarget resource views */
	ID3D11ShaderResourceView* ShaderResView;
	ID3D11RenderTargetView* RenderTargetView;

	void ReleaseAll()
	{
		if(Texture)Texture->Release(); Texture=NULL;
		if(ShaderResView)ShaderResView->Release(); ShaderResView=NULL;
		if(RenderTargetView)RenderTargetView->Release(); RenderTargetView=NULL;
	}
};



/** Struct for a texture that can be used as shader resource AND depth stencil target */
struct RenderToDepthStencilBuffer
{
	~RenderToDepthStencilBuffer()
	{
		ReleaseAll();
	}

	/** Creates the render-to-texture buffers */
	RenderToDepthStencilBuffer(ID3D11Device* device, UINT SizeX,UINT SizeY,DXGI_FORMAT Format, HRESULT* Result=NULL, DXGI_FORMAT DSVFormat=DXGI_FORMAT_UNKNOWN, DXGI_FORMAT SRVFormat=DXGI_FORMAT_UNKNOWN)
	{
		HRESULT hr=S_OK;

		Texture=NULL;
		ShaderResView=NULL;
		DepthStencilView=NULL;

		if(SizeX == 0 || SizeY == 0)
		{
			LogError() << L"SizeX or SizeY can't be 0";
		}

		this->SizeX=SizeX;
		this->SizeY=SizeY;

		if(Format==0)
		{
			LogError() << L"DXGI_FORMAT_UNKNOWN (0) isn't a valid texture format";
		}

		//Create a new render target texture
		D3D11_TEXTURE2D_DESC Desc;
		ZeroMemory( &Desc, sizeof( D3D10_TEXTURE2D_DESC ) );
		Desc.ArraySize = 1;
		Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.Format = Format;
		Desc.Width = SizeX;
		Desc.Height = SizeY;
		Desc.MipLevels = 1;
		Desc.SampleDesc.Count = 1;


		LE(device->CreateTexture2D(&Desc,NULL,&Texture));

		//Create a render target view
		D3D11_DEPTH_STENCIL_VIEW_DESC DescDSV;
		ZeroMemory( &DescDSV, sizeof( DescDSV ) );
		DescDSV.Format = (DSVFormat != DXGI_FORMAT_UNKNOWN ? DSVFormat : Desc.Format);
		DescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		DescDSV.Texture2D.MipSlice = 0;
		DescDSV.Flags=0;
		LE(device->CreateDepthStencilView((ID3D11Resource *)Texture, &DescDSV, &DepthStencilView));

		
		// Create the resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC DescRV;
		DescRV.Format = (SRVFormat != DXGI_FORMAT_UNKNOWN ? SRVFormat : Desc.Format);
		DescRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		DescRV.Texture2D.MipLevels = 1;
		DescRV.Texture2D.MostDetailedMip = 0;
		LE(device->CreateShaderResourceView( (ID3D11Resource *)Texture, &DescRV, &ShaderResView ));
		
		if(FAILED(hr))
		{
			LogError() << L"Coould not create ID3D11Texture2D, ID3D11ShaderResourceView, or ID3D11DepthStencilView. Killing created resources (If any).";
			ReleaseAll();
			if(Result)*Result=hr;
			return;
		}

		//LogInfo() << L"RenderToDepthStencilStruct: Successfully created ID3D11Texture2D, ID3D11ShaderResourceView, and ID3D11DepthStencilView.";
		if(Result)*Result=hr;
	}

	void BindToPixelShader(ID3D11DeviceContext* context, int slot)
	{
		context->PSSetShaderResources(slot, 1, &ShaderResView);
	};

	ID3D11Texture2D* GetTexture(){return Texture;}
	ID3D11ShaderResourceView* GetShaderResView(){return ShaderResView;}
	ID3D11DepthStencilView* GetDepthStencilView(){return DepthStencilView;}
	UINT GetSizeX(){return SizeX;}
	UINT GetSizeY(){return SizeY;}
private:

	// The Texture object
	ID3D11Texture2D* Texture;

	UINT SizeX;
	UINT SizeY;

	// Shader and rendertarget resource views
	ID3D11ShaderResourceView* ShaderResView;
	ID3D11DepthStencilView* DepthStencilView;

	void ReleaseAll()
	{
		if(Texture)Texture->Release(); Texture=NULL;
		if(ShaderResView)ShaderResView->Release(); ShaderResView=NULL;
		if(DepthStencilView)DepthStencilView->Release(); DepthStencilView=NULL;
	}
};