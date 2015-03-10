/***************************************************************
* Project: DDrawWrap
* File: IDirectDrawSurface7.cpp
* Copyright © learn_more
*/
#pragma once
#include "../pch.h"
#include <ddraw.h>
#include <d3d11.h>

class MyDirectDrawSurface7;
class FakeDirectDrawSurface7 : public IDirectDrawSurface7
{
public:
	FakeDirectDrawSurface7();

	/*** IUnknown methods ***/
	HRESULT __stdcall QueryInterface( REFIID riid, LPVOID* ppvObj );
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	/*** IDirectDraw methods ***/
	HRESULT __stdcall AddAttachedSurface( LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface );
	HRESULT __stdcall AddOverlayDirtyRect( LPRECT lpRect );
	HRESULT __stdcall Blt( LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx );
	HRESULT __stdcall BltBatch( LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags );
	HRESULT __stdcall BltFast( DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans );
	HRESULT __stdcall DeleteAttachedSurface( DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface );
	HRESULT __stdcall EnumAttachedSurfaces( LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback );
	HRESULT __stdcall EnumOverlayZOrders( DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback );
	HRESULT __stdcall Flip( LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags );
	HRESULT __stdcall GetAttachedSurface( LPDDSCAPS2 lpDDSCaps, LPDIRECTDRAWSURFACE7* lplpDDAttachedSurface );
	HRESULT __stdcall GetBltStatus( DWORD dwFlags );
	HRESULT __stdcall GetCaps( LPDDSCAPS2 lpDDSCaps );
	HRESULT __stdcall GetClipper( LPDIRECTDRAWCLIPPER* lplpDDClipper );
	HRESULT __stdcall GetColorKey( DWORD dwFlags, LPDDCOLORKEY lpDDColorKey );
	HRESULT __stdcall GetDC( HDC* lphDC );
	HRESULT __stdcall GetFlipStatus( DWORD dwFlags );
	HRESULT __stdcall GetOverlayPosition( LPLONG lplX, LPLONG lplY );
	HRESULT __stdcall GetPalette( LPDIRECTDRAWPALETTE* lplpDDPalette );
	HRESULT __stdcall GetPixelFormat( LPDDPIXELFORMAT lpDDPixelFormat );
	HRESULT __stdcall GetSurfaceDesc( LPDDSURFACEDESC2 lpDDSurfaceDesc );
	HRESULT __stdcall Initialize( LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc );
	HRESULT __stdcall IsLost();
	HRESULT __stdcall Lock( LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent );
	HRESULT __stdcall ReleaseDC( HDC hDC );
	HRESULT __stdcall Restore();
	HRESULT __stdcall SetClipper( LPDIRECTDRAWCLIPPER lpDDClipper );
	HRESULT __stdcall SetColorKey( DWORD dwFlags, LPDDCOLORKEY lpDDColorKey );
	HRESULT __stdcall SetOverlayPosition( LONG lX, LONG lY );
	HRESULT __stdcall SetPalette( LPDIRECTDRAWPALETTE lpDDPalette );
	HRESULT __stdcall Unlock( LPRECT lpRect );
	HRESULT __stdcall UpdateOverlay( LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx );
	HRESULT __stdcall UpdateOverlayDisplay( DWORD dwFlags );
	HRESULT __stdcall UpdateOverlayZOrder( DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference );
	/*** Added in the V2 Interface ***/
	HRESULT __stdcall GetDDInterface( LPVOID* lplpDD );
	HRESULT __stdcall PageLock( DWORD dwFlags );
	HRESULT __stdcall PageUnlock( DWORD dwFlags );
	/*** Added in the V3 Interface ***/
	HRESULT __stdcall SetSurfaceDesc( LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags );
	/*** Added in the V4 Interface ***/
	HRESULT __stdcall SetPrivateData( REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags );
	HRESULT __stdcall GetPrivateData( REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize );
	HRESULT __stdcall FreePrivateData( REFGUID guidTag );
	HRESULT __stdcall GetUniquenessValue( LPDWORD lpValue );
	HRESULT __stdcall ChangeUniquenessValue();
	/*** Moved Texture7 methods here ***/
	HRESULT __stdcall SetPriority( DWORD dwPriority );
	HRESULT __stdcall GetPriority( LPDWORD dwPriority );
	HRESULT __stdcall SetLOD( DWORD dwLOD );
	HRESULT __stdcall GetLOD( LPDWORD dwLOD );

	void InitFakeSurface(const DDSURFACEDESC2* desc, MyDirectDrawSurface7* resource, int mipLevel);
private:

	/** Current ref-count */
	int RefCount;

	/** Mip-level this represents */
	int MipLevel;

	/** Data pointer of this surface (Only valid during lock)*/
	unsigned char* Data;

	/** Array of further attached surfaces */
	std::vector<FakeDirectDrawSurface7 *> AttachedSurfaces;

	/** The original desc this was created with */
	DDSURFACEDESC2 OriginalDesc;

	/** The base resource */
	MyDirectDrawSurface7* Resource;
};
