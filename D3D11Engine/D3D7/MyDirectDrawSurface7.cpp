#include "../pch.h"
#include "MyDirectDrawSurface7.h"
#include <ddraw.h>
#include <d3d11.h>
#include "../Engine.h"
#include "../GothicAPI.h"
#include "../BaseGraphicsEngine.h"
#include "../BaseTexture.h"
#include "../zCTexture.h"

#define DebugWriteTex(x)  DebugWrite(x)

MyDirectDrawSurface7::MyDirectDrawSurface7()
{
	refCount = 1;
	EngineTexture = NULL;
	Normalmap = NULL;
	FxMap = NULL;
	LockedData = NULL;
	GothicTexture = NULL;
	IsReady = false;

	LockType = 0;

	// Check for test-bind mode to figure out what zCTexture-Object we are associated with
	std::string bound;
	if(Engine::GAPI->IsInTextureTestBindMode(bound))
	{
		Engine::GAPI->SetTextureTestBindMode(false, "");
		return;
	}
}

MyDirectDrawSurface7::~MyDirectDrawSurface7()
{
	Engine::GAPI->RemoveSurface(this);

	// Sometimes gothic doesn't unlock a surface or this is a movie-buffer
	delete[] LockedData;

	delete EngineTexture;
	delete Normalmap;
	delete FxMap;
}

/** Returns the engine texture of this surface */
BaseTexture* MyDirectDrawSurface7::GetEngineTexture()
{
	return EngineTexture;
}

/** Returns the engine texture of this surface */
BaseTexture* MyDirectDrawSurface7::GetNormalmap()
{
	return Normalmap;
}

/** Returns the fx-map for this surface */
BaseTexture* MyDirectDrawSurface7::GetFxMap()
{
	return FxMap;
}

/** Binds this texture */
void MyDirectDrawSurface7::BindToSlot(int slot)
{
	if(!IsReady)
		return; // Don't bind half-loaded textures!

	if(EngineTexture) // Needed sometimes
		EngineTexture->BindToPixelShader(slot);

	if(Normalmap)
	{
		Normalmap->BindToPixelShader(slot + 1);
		Normalmap->BindToVertexShader(0);
	}else
	{
		//EngineTexture->BindToPixelShader(slot + 1);
		Engine::GraphicsEngine->UnbindTexture(slot + 1);
	}
}

/** Loads additional resources if possible */
void MyDirectDrawSurface7::LoadAdditionalResources(zCTexture* ownedTexture)
{
	GothicTexture = ownedTexture;
	TextureName = GothicTexture->GetNameWithoutExt();	

	Engine::GAPI->AddSurface(TextureName, this);

	if(Normalmap)
	{
		delete Normalmap;
		Normalmap = NULL;
	}

	if(FxMap)
	{
		delete FxMap;
		FxMap = NULL;
	}

	if(!TextureName.size() || Normalmap || FxMap)
		return;

	std::string normalmap = "system\\GD3D11\\textures\\replacements\\" + TextureName + "_normal.dds";
	FILE* f = fopen(normalmap.c_str(), "rb");
	BaseTexture* nrmmapTexture = NULL;
	if(f)
	{
		// Create the texture object this is linked with
		
		Engine::GraphicsEngine->CreateTexture(&nrmmapTexture);
	
		if(XR_SUCCESS != nrmmapTexture->Init(normalmap))
		{
			delete nrmmapTexture;
			nrmmapTexture = NULL;
			LogWarn() << "Failed to load normalmap!";
		}
	}

	std::string fxMap = "system\\GD3D11\\textures\\replacements\\" + TextureName + "_fx.dds";
	f = fopen(fxMap.c_str(), "rb");
	BaseTexture* fxMapTexture = NULL;
	if(f)
	{
		// Create the texture object this is linked with
		Engine::GraphicsEngine->CreateTexture(&fxMapTexture);
	
		if(XR_SUCCESS != fxMapTexture->Init(fxMap))
		{
			delete fxMapTexture;
			fxMapTexture = NULL;
			LogWarn() << "Failed to load normalmap!";
		}
	}



	Normalmap = nrmmapTexture;
	FxMap = fxMapTexture;
}

HRESULT MyDirectDrawSurface7::QueryInterface( REFIID riid, LPVOID* ppvObj )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::QueryInterface( %s )");
	return S_OK; 
}

ULONG MyDirectDrawSurface7::AddRef()
{
	DebugWriteTex("IDirectDrawSurface7(%p)::AddRef( %i )");

	refCount++;
	return refCount;
}

ULONG MyDirectDrawSurface7::Release()
{
	refCount--;
	ULONG uRet = refCount;
	DebugWriteTex("IDirectDrawSurface7(%p)::Release( %i )");

	if(uRet == 0)
	{
		delete this;
	}

	return uRet;
}

HRESULT MyDirectDrawSurface7::AddAttachedSurface( LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::AddAttachedSurface()");
	attachedSurfaces.push_back((MyDirectDrawSurface7 *)lpDDSAttachedSurface);
	return S_OK;
}

HRESULT MyDirectDrawSurface7::AddOverlayDirtyRect( LPRECT lpRect )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::AddOverlayDirtyRect()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::Blt( LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::Blt()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::BltBatch( LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::BltBatch()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::BltFast( DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::BltFast()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::DeleteAttachedSurface( DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::DeleteAttachedSurface()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::EnumAttachedSurfaces( LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::EnumAttachedSurfaces()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::EnumOverlayZOrders( DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::EnumOverlayZOrders()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::Flip( LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::Flip() #####");

	return S_OK; 
}

HRESULT MyDirectDrawSurface7::GetAttachedSurface( LPDDSCAPS2 lpDDSCaps2, LPDIRECTDRAWSURFACE7* lplpDDAttachedSurface )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetAttachedSurface()");

	if(attachedSurfaces.empty())
		return E_FAIL;

	*lplpDDAttachedSurface = attachedSurfaces[0];
	attachedSurfaces[0]->AddRef();

	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetBltStatus( DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetBltStatus()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetCaps( LPDDSCAPS2 lpDDSCaps2 )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetCaps()");
	*lpDDSCaps2 = OriginalSurfaceDesc.ddsCaps;

	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetClipper( LPDIRECTDRAWCLIPPER* lplpDDClipper )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetClipper()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetColorKey( DWORD dwFlags, LPDDCOLORKEY lpDDColorKey )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetColorKey()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetDC( HDC* lphDC )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetDC()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetFlipStatus( DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetFlipStatus()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::GetOverlayPosition( LPLONG lplX, LPLONG lplY )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetOverlayPosition()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::GetPalette( LPDIRECTDRAWPALETTE* lplpDDPalette )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetPalette()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetPixelFormat( LPDDPIXELFORMAT lpDDPixelFormat )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetPixelFormat()");

	*lpDDPixelFormat = OriginalSurfaceDesc.ddpfPixelFormat;

	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetSurfaceDesc( LPDDSURFACEDESC2 lpDDSurfaceDesc )
{
	*lpDDSurfaceDesc = OriginalSurfaceDesc;
	return S_OK;
}

HRESULT MyDirectDrawSurface7::Initialize( LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::Initialize()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::IsLost()
{
	DebugWriteTex("IDirectDrawSurface7(%p)::IsLost()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::Lock( LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::Lock()");

	LockType = dwFlags;

	*lpDDSurfaceDesc = OriginalSurfaceDesc;

	// This has to be a backbuffer-copy
	if((LockType & DDLOCK_READONLY) != 0 && LockType != DDLOCK_READONLY) // Gothic uses DDLOCK_READONLY + some other flags for getting the framebuffer. DDLOCK_READONLY only is for movie playback. 
	{
		// Assume 32-bit
		byte* data;
		int pixelSize;
		Engine::GraphicsEngine->GetBackbufferData(&data, pixelSize);

		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount = 32;
		lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask = 0x000000ff;

		lpDDSurfaceDesc->lPitch = Engine::GraphicsEngine->GetBackbufferResolution().x * pixelSize;
		lpDDSurfaceDesc->dwWidth = Engine::GraphicsEngine->GetBackbufferResolution().x;
		lpDDSurfaceDesc->dwHeight = Engine::GraphicsEngine->GetBackbufferResolution().y;
		lpDDSurfaceDesc->lpSurface = data;

		LockedData = data;

		return S_OK;
	}

	if(!EngineTexture)
		return S_OK;

	// Check for 16-bit surface. We allocate the texture as 32-bit, so we need to divide the size by two for that
	int redBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwRBitMask);
	int greenBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwGBitMask);
	int blueBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwBBitMask);
	int alphaBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask);

	int bpp = redBits + greenBits + blueBits + alphaBits;
	int divisor = 1;

	if(bpp == 16)
		divisor = 2;

	if(bpp == 24)
	{
		// Handle movie frame,
		// don't deallocate the memory after unlock, since only the changing parts in videos will get updated
		if(!LockedData)
			LockedData = new unsigned char[EngineTexture->GetSizeInBytes(0) / divisor];

	}else
	{
		// Allocate some temporary data
		LockedData = new unsigned char[EngineTexture->GetSizeInBytes(0) / divisor];
	}

	lpDDSurfaceDesc->lpSurface = LockedData;
	lpDDSurfaceDesc->lPitch = EngineTexture->GetRowPitchBytes(0) / divisor;

	return S_OK;
}

#include "../squish-1.11/squish.h"
#include "../lodepng.h"

HRESULT MyDirectDrawSurface7::Unlock( LPRECT lpRect )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::Unlock()");

	// This has to be a backbuffer-copy
	if((LockType & DDLOCK_READONLY) != 0 && LockType != DDLOCK_READONLY)
	{
		// Clean up
		delete[] LockedData;
		LockedData = NULL;

		return S_OK;
	}

	if(Engine::GAPI->GetBoundTexture(7) != NULL)
	{
		// Comming from LoadResourceData
		LoadAdditionalResources(Engine::GAPI->GetBoundTexture(7));
	}

	// If this is a 16-bit surface, we need to convert it to 32-bit first
	int redBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwRBitMask);
	int greenBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwGBitMask);
	int blueBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwBBitMask);
	int alphaBits = Toolbox::GetNumberOfBits(OriginalSurfaceDesc.ddpfPixelFormat.dwRGBAlphaBitMask);

	int bpp = redBits + greenBits + blueBits + alphaBits;

	if(bpp == 16)
	{
		// Convert
		unsigned char* dst = new unsigned char[EngineTexture->GetSizeInBytes(0)];
		for(unsigned int i=0;i<EngineTexture->GetSizeInBytes(0) / 4;i++)
		{
			unsigned char temp0 = LockedData[i * 2 + 0];
			unsigned char temp1 = LockedData[i * 2 + 1];
			unsigned pixel_data = temp1 << 8 | temp0;

			unsigned char blueComponent  = (pixel_data & 0x1F);
			unsigned char greenComponent = (pixel_data >> 5 ) & 0x3F;
			unsigned char redComponent   = (pixel_data >> 11) & 0x1F;

			// Extract red, green and blue components from the 16 bits
			dst[4*i+0] = (unsigned char)((redComponent  / 32.0) * 255.0f);
			dst[4*i+1] = (unsigned char)((greenComponent  / 64.0) * 255.0f);
			dst[4*i+2] = (unsigned char)((blueComponent  / 32.0) * 255.0f);
			dst[4*i+3] = 255;
		}

		if(Engine::GAPI->GetMainThreadID() != GetCurrentThreadId())
		{
			EngineTexture->UpdateDataDeferred(dst, 0);
			Engine::GAPI->AddFrameLoadedTexture(this);
		}
		else
		{
			EngineTexture->UpdateData(dst, 0);
			SetReady(true);
		}

		delete[] dst;

		//Engine::GAPI->EnterResourceCriticalSection();
		EngineTexture->GenerateMipMaps();
		//Engine::GAPI->LeaveResourceCriticalSection();
	}else
	{
		/*unsigned char* convertedData = new unsigned char[OriginalSurfaceDesc.dwWidth * OriginalSurfaceDesc.dwHeight * 4];
		ZeroMemory(convertedData, OriginalSurfaceDesc.dwWidth * OriginalSurfaceDesc.dwHeight * 4);

		static int num = 0;
		num++;


		// DDS-Texture
		if((OriginalSurfaceDesc.ddpfPixelFormat.dwFlags & DDPF_FOURCC) == DDPF_FOURCC)
		{
			switch(OriginalSurfaceDesc.ddpfPixelFormat.dwFourCC)
			{
			case FOURCC_DXT1:
				squish::DecompressImage(convertedData, OriginalSurfaceDesc.dwWidth, OriginalSurfaceDesc.dwHeight, LockedData, squish::kDxt1);
				break;

			case FOURCC_DXT2:
			case FOURCC_DXT3:
				squish::DecompressImage(convertedData, OriginalSurfaceDesc.dwWidth, OriginalSurfaceDesc.dwHeight, LockedData, squish::kDxt3);
				break;

			case FOURCC_DXT4:
			case FOURCC_DXT5:
				squish::DecompressImage(convertedData, OriginalSurfaceDesc.dwWidth, OriginalSurfaceDesc.dwHeight, LockedData, squish::kDxt5);
				break;

			default:
				LogErrorBox() << "Invalid DXT-Format!";
			}

			lodepng::State s;
			lodepng::encode((std::string("tex_") + std::to_string(num) + ".png").c_str(), convertedData, OriginalSurfaceDesc.dwWidth, OriginalSurfaceDesc.dwHeight);

			delete[] convertedData;
			
		}*/

		

		if(bpp == 24)
		{
			/*unsigned char* dst = new unsigned char[OriginalSurfaceDesc.dwWidth * OriginalSurfaceDesc.dwHeight * 4];

			// Convert from 24 bits to 32
			for(unsigned int i=0;i<OriginalSurfaceDesc.dwWidth * OriginalSurfaceDesc.dwHeight;i++)
			{
				unsigned char blueComponent  = LockedData[i * 3 + 2];
				unsigned char greenComponent = LockedData[i * 3 + 1];
				unsigned char redComponent   = LockedData[i * 3 + 0];

				dst[4*i+0] = redComponent;
				dst[4*i+1] = greenComponent;
				dst[4*i+2] = blueComponent;
				dst[4*i+3] = 255;
			}

			

			delete[] dst;*/

			// This is a movie frame, draw it to the sceen
			EngineTexture->UpdateData(LockedData, 0);

			EngineTexture->BindToPixelShader(0);
			Engine::GAPI->GetRendererState()->BlendState.SetDefault();
			Engine::GAPI->GetRendererState()->BlendStateDirty = true;
			Engine::GraphicsEngine->DrawQuad(INT2(0,0), Engine::GraphicsEngine->GetResolution());
		}else
		{
			// No conversion needed
			if(Engine::GAPI->GetMainThreadID() != GetCurrentThreadId())
			{
				EngineTexture->UpdateDataDeferred(LockedData, 0);
				Engine::GAPI->AddFrameLoadedTexture(this);
			}
			else
			{
				EngineTexture->UpdateData(LockedData, 0);
				SetReady(true);
			}
		}
	}

	if(bpp != 24)
	{
		// Clean up if not a movie frame
		delete[] LockedData;
		LockedData = NULL;
	}

	return S_OK;
}


HRESULT MyDirectDrawSurface7::ReleaseDC( HDC hDC )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::ReleaseDC()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::Restore()
{
	DebugWriteTex("IDirectDrawSurface7(%p)::Restore()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::SetClipper( LPDIRECTDRAWCLIPPER lpDDClipper )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetClipper()");
	hook_infunc


	HWND hWnd;
	lpDDClipper->GetHWnd(&hWnd);
	Engine::GAPI->OnSetWindow(hWnd);

	hook_outfunc

	return S_OK;
}

HRESULT MyDirectDrawSurface7::SetColorKey( DWORD dwFlags, LPDDCOLORKEY lpDDColorKey )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetColorKey( %s, %s )");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::SetOverlayPosition( LONG lX, LONG lY )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetOverlayPosition()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::SetPalette( LPDIRECTDRAWPALETTE lpDDPalette )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetPalette()");
	return S_OK;
}


HRESULT MyDirectDrawSurface7::UpdateOverlay( LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::UpdateOverlay()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::UpdateOverlayDisplay( DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::UpdateOverlayDisplay()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::UpdateOverlayZOrder( DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::UpdateOverlayZOrder()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::GetDDInterface( LPVOID* lplpDD )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetDDInterface()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::PageLock( DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::PageLock()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::PageUnlock( DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::PageUnlock()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::SetSurfaceDesc( LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetSurfaceDesc()");

	OriginalSurfaceDesc = *lpDDSurfaceDesc;

	// Check if this is the rendertarget or something else we dont need
	if(lpDDSurfaceDesc->dwWidth == 0)
	{
		return S_OK;
	}

	// Create the texture object this is linked with
	Engine::GraphicsEngine->CreateTexture(&EngineTexture);

	
	int redBits = Toolbox::GetNumberOfBits(lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask);
	int greenBits = Toolbox::GetNumberOfBits(lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask);
	int blueBits = Toolbox::GetNumberOfBits(lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask);
	int alphaBits = Toolbox::GetNumberOfBits(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask);

	int bpp = redBits + greenBits + blueBits + alphaBits;

	// Find out format
	BaseTexture::ETextureFormat format = BaseTexture::ETextureFormat::TF_R8G8B8A8;
	switch(bpp)
	{
	case 16:
		format = BaseTexture::ETextureFormat::TF_R8G8B8A8;
		break;

	case 24:
	case 32:
		format = BaseTexture::ETextureFormat::TF_R8G8B8A8;

	case 0:
		{
			// DDS-Texture
			if((lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_FOURCC) == DDPF_FOURCC)
			{
				switch(lpDDSurfaceDesc->ddpfPixelFormat.dwFourCC)
				{
				case FOURCC_DXT1:
					format = BaseTexture::ETextureFormat::TF_DXT1;
					break;

				case FOURCC_DXT2:
				case FOURCC_DXT3:
					format = BaseTexture::ETextureFormat::TF_DXT3;
					break;

				case FOURCC_DXT4:
				case FOURCC_DXT5:
					format = BaseTexture::ETextureFormat::TF_DXT5;
					break;
				}
			}
		}
		break;
	}

	// Find out mip-level count
	unsigned int mipMapCount = 1;
	if(lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_MIPMAP)
	{
		mipMapCount = lpDDSurfaceDesc->dwMipMapCount;
	}

	// Create the texture
	EngineTexture->Init(INT2(lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight), format, mipMapCount, NULL);

	return S_OK;
}

HRESULT MyDirectDrawSurface7::SetPrivateData( REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetPrivateData()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::GetPrivateData( REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetPrivateData()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::FreePrivateData( REFGUID guidTag )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::FreePrivateData()");
	return S_OK;
}

HRESULT MyDirectDrawSurface7::GetUniquenessValue( LPDWORD lpValue )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetUniquenessValue()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::ChangeUniquenessValue()
{
	DebugWriteTex("IDirectDrawSurface7(%p)::ChangeUniquenessValue()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::SetPriority( DWORD dwPriority )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetPriority()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::GetPriority( LPDWORD dwPriority )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetPriority()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::SetLOD( DWORD dwLOD )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::SetLOD()");
	return S_OK; 
}

HRESULT MyDirectDrawSurface7::GetLOD( LPDWORD dwLOD )
{
	DebugWriteTex("IDirectDrawSurface7(%p)::GetLOD()");
	return S_OK; 
}

/** Returns the name of this surface */
const std::string& MyDirectDrawSurface7::GetTextureName()
{
	return TextureName;
}