#include "../pch.h"
#include "FakeDirectDrawSurface7.h"
#include <ddraw.h>
#include <d3d11.h>
#include "MyDirectDrawSurface7.h"
#include "../D3D11Texture.h"
#include "../Engine.h"
#include "../GothicAPI.h"


FakeDirectDrawSurface7::FakeDirectDrawSurface7()
{
	RefCount = 1;
	Data = NULL;
}

void FakeDirectDrawSurface7::InitFakeSurface(const DDSURFACEDESC2* desc, MyDirectDrawSurface7* Resource, int mipLevel)
{
	OriginalDesc = *desc;
	this->Resource = Resource;
	MipLevel = mipLevel;
}

HRESULT FakeDirectDrawSurface7::QueryInterface( REFIID riid, LPVOID* ppvObj )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::QueryInterface( %s )");//, PRINT_DEV, buf );
	return S_OK; //return originalSurface->QueryInterface( riid, ppvObj );
}

ULONG FakeDirectDrawSurface7::AddRef()
{
	RefCount++;
	DebugWrite("FakeDirectDrawSurface7(%p)::AddRef( %i )");//, PRINT_DEV, pRef );
	return RefCount;
}

ULONG FakeDirectDrawSurface7::Release()
{
	
	DebugWrite("FakeDirectDrawSurface7(%p)::Release( %i )");//, PRINT_DEV, uRet );
	RefCount--;

	if(RefCount == 0)
	{
		delete[] Data;
		delete this;
		return 0;
	}

	return RefCount;
}

HRESULT FakeDirectDrawSurface7::AddAttachedSurface( LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::AddAttachedSurface()");//, PRINT_DEV );
	AttachedSurfaces.push_back((FakeDirectDrawSurface7 *)lpDDSAttachedSurface);
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::AddOverlayDirtyRect( LPRECT lpRect )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::AddOverlayDirtyRect()");//, PRINT_DEV );
	return S_OK; //return originalSurface->AddOverlayDirtyRect( lpRect );
}

HRESULT FakeDirectDrawSurface7::Blt( LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::Blt()");//, PRINT_DEV );

	// Gothic never really blts


	return S_OK; //return originalSurface->Blt( lpDestRect, ((FakeDirectDrawSurface7 *)lpDDSrcSurface)->GetOriginalSurface(), lpSrcRect, dwFlags, lpDDBltFx );
}

HRESULT FakeDirectDrawSurface7::BltBatch( LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::BltBatch()");//, PRINT_DEV );
	return S_OK; //return originalSurface->BltBatch( lpDDBltBatch, dwCount, dwFlags );
}

HRESULT FakeDirectDrawSurface7::BltFast( DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::BltFast()");//, PRINT_DEV );
	return S_OK; //return originalSurface->BltFast( dwX, dwY, lpDDSrcSurface, lpSrcRect, dwTrans );
}

HRESULT FakeDirectDrawSurface7::DeleteAttachedSurface( DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::DeleteAttachedSurface()");//, PRINT_DEV );
	return S_OK; //return originalSurface->DeleteAttachedSurface( dwFlags, lpDDSAttachedSurface );
}

HRESULT FakeDirectDrawSurface7::EnumAttachedSurfaces( LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::EnumAttachedSurfaces()");//, PRINT_DEV );
	return S_OK; //return originalSurface->EnumAttachedSurfaces( lpContext, lpEnumSurfacesCallback );
}

HRESULT FakeDirectDrawSurface7::EnumOverlayZOrders( DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::EnumOverlayZOrders()");//, PRINT_DEV );
	return S_OK; //return originalSurface->EnumOverlayZOrders( dwFlags, lpContext, lpfnCallback );
}

HRESULT FakeDirectDrawSurface7::Flip( LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::Flip() #####");//, PRINT_DEV );

	//transferFrame(this);

	//return S_OK; //return originalSurface->Flip( lpDDSurfaceTargetOverride, dwFlags );
	return S_OK; // Dont actually flip this
}

HRESULT FakeDirectDrawSurface7::GetAttachedSurface( LPDDSCAPS2 lpDDSCaps2, LPDIRECTDRAWSURFACE7* lplpDDAttachedSurface )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetAttachedSurface()");//, PRINT_DEV );
	//return originalSurface->GetAttachedSurface( lpDDSCaps2, lplpDDAttachedSurface );

	if(AttachedSurfaces.empty())
		return E_FAIL;

	*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE7)AttachedSurfaces[0]; // Mipmap chains only have one entry
	AttachedSurfaces[0]->AddRef();
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::GetBltStatus( DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetBltStatus()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetBltStatus( dwFlags );
}

HRESULT FakeDirectDrawSurface7::GetCaps( LPDDSCAPS2 lpDDSCaps2 )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetCaps()");//, PRINT_DEV );
	*lpDDSCaps2 = OriginalDesc.ddsCaps;

	return S_OK; //return originalSurface->GetCaps( lpDDSCaps2 );
}

HRESULT FakeDirectDrawSurface7::GetClipper( LPDIRECTDRAWCLIPPER* lplpDDClipper )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetClipper()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetClipper( lplpDDClipper );
}

HRESULT FakeDirectDrawSurface7::GetColorKey( DWORD dwFlags, LPDDCOLORKEY lpDDColorKey )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetColorKey()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetColorKey( dwFlags, lpDDColorKey );
}

HRESULT FakeDirectDrawSurface7::GetDC( HDC* lphDC )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetDC()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetDC( lphDC );
}

HRESULT FakeDirectDrawSurface7::GetFlipStatus( DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetFlipStatus()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetFlipStatus( dwFlags );
}

HRESULT FakeDirectDrawSurface7::GetOverlayPosition( LPLONG lplX, LPLONG lplY )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetOverlayPosition()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetOverlayPosition( lplX, lplY );
}

HRESULT FakeDirectDrawSurface7::GetPalette( LPDIRECTDRAWPALETTE* lplpDDPalette )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetPalette()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetPalette( lplpDDPalette );
}

HRESULT FakeDirectDrawSurface7::GetPixelFormat( LPDDPIXELFORMAT lpDDPixelFormat )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetPixelFormat()");//, PRINT_DEV );
	return S_OK; //return originalSurface->GetPixelFormat( lpDDPixelFormat );
}

HRESULT FakeDirectDrawSurface7::GetSurfaceDesc( LPDDSURFACEDESC2 lpDDSurfaceDesc )
{
	*lpDDSurfaceDesc = OriginalDesc;
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::Initialize( LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::Initialize()");//, PRINT_DEV );
	return S_OK; //return originalSurface->Initialize( lpDD, lpDDSurfaceDesc );
}

HRESULT FakeDirectDrawSurface7::IsLost()
{
	DebugWrite("FakeDirectDrawSurface7(%p)::IsLost()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::Lock( LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::Lock( %s, %s )");
	*lpDDSurfaceDesc = OriginalDesc;

	// Allocate some temporary data
	Data = new unsigned char[Resource->GetEngineTexture()->GetSizeInBytes(MipLevel)];
	lpDDSurfaceDesc->lpSurface = Data;
	lpDDSurfaceDesc->lPitch = Resource->GetEngineTexture()->GetRowPitchBytes(MipLevel);

	int px = (int)std::max(1.0f, floor(OriginalDesc.dwWidth / pow(2.0f, MipLevel)));
	int py = (int)std::max(1.0f, floor(OriginalDesc.dwHeight / pow(2.0f, MipLevel)));

	lpDDSurfaceDesc->dwWidth = px;
	lpDDSurfaceDesc->dwHeight = py;

	return S_OK;
}

#include "../lodepng.h"
HRESULT FakeDirectDrawSurface7::Unlock( LPRECT lpRect )
{
	DebugWrite("FakeDirectDrawSurface7::Unlock");

	int redBits = Toolbox::GetNumberOfBits(OriginalDesc.ddpfPixelFormat.dwRBitMask);
	int greenBits = Toolbox::GetNumberOfBits(OriginalDesc.ddpfPixelFormat.dwGBitMask);
	int blueBits = Toolbox::GetNumberOfBits(OriginalDesc.ddpfPixelFormat.dwBBitMask);
	int alphaBits = Toolbox::GetNumberOfBits(OriginalDesc.ddpfPixelFormat.dwRGBAlphaBitMask);

	int bpp = redBits + greenBits + blueBits + alphaBits;

	if(bpp == 16)
	{
		// Convert
		/*unsigned char* dst = new unsigned char[Resource->GetEngineTexture()->GetSizeInBytes(MipLevel)];
		for(unsigned int i=0;i<Resource->GetEngineTexture()->GetSizeInBytes(MipLevel) / 4;i++)
		{
			unsigned char temp0 = Data[i * 2 + 0];
			unsigned char temp1 = Data[i * 2 + 1];
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

		int px = (int)std::max(1.0f, floor(OriginalDesc.dwWidth / pow(2.0f, MipLevel)));
		int py = (int)std::max(1.0f, floor(OriginalDesc.dwHeight / pow(2.0f, MipLevel)));
		lodepng::encode(Resource->GetTextureName() + "_" + std::to_string(MipLevel) + ".png", dst, px, py);

		Resource->GetEngineTexture()->UpdateDataDeferred(dst, MipLevel);

		delete[] dst;*/
	}else
	{
		Resource->GetEngineTexture()->UpdateDataDeferred(Data, MipLevel);
		Resource->IncreaseQueuedMipMapCount();
		Engine::GAPI->AddFrameLoadedTexture(Resource);
	}

	delete[] Data;
	Data = NULL;

	return S_OK;
}

HRESULT FakeDirectDrawSurface7::ReleaseDC( HDC hDC )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::ReleaseDC()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::Restore()
{
	DebugWrite("FakeDirectDrawSurface7(%p)::Restore()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::SetClipper( LPDIRECTDRAWCLIPPER lpDDClipper )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::SetClipper()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::SetColorKey( DWORD dwFlags, LPDDCOLORKEY lpDDColorKey )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::SetColorKey( %s, %s )");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::SetOverlayPosition( LONG lX, LONG lY )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::SetOverlayPosition()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::SetPalette( LPDIRECTDRAWPALETTE lpDDPalette )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::SetPalette()");
	return S_OK; 
}



HRESULT FakeDirectDrawSurface7::UpdateOverlay( LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::UpdateOverlay()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::UpdateOverlayDisplay( DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::UpdateOverlayDisplay()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::UpdateOverlayZOrder( DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::UpdateOverlayZOrder()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::GetDDInterface( LPVOID* lplpDD )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetDDInterface()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::PageLock( DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::PageLock()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::PageUnlock( DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::PageUnlock()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::SetSurfaceDesc( LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::PageUnlock()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::SetPrivateData( REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::SetPrivateData()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::GetPrivateData( REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetPrivateData()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::FreePrivateData( REFGUID guidTag )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::FreePrivateData()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::GetUniquenessValue( LPDWORD lpValue )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetUniquenessValue()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::ChangeUniquenessValue()
{
	DebugWrite("FakeDirectDrawSurface7(%p)::ChangeUniquenessValue()");
	return S_OK; 
}

HRESULT FakeDirectDrawSurface7::SetPriority( DWORD dwPriority )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::SetPriority()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::GetPriority( LPDWORD dwPriority )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetPriority()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::SetLOD( DWORD dwLOD )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::SetLOD()");
	return S_OK;
}

HRESULT FakeDirectDrawSurface7::GetLOD( LPDWORD dwLOD )
{
	DebugWrite("FakeDirectDrawSurface7(%p)::GetLOD()");
	return S_OK;
}
