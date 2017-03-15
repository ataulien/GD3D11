/***************************************************************
* Project: DDrawWrap
* File: IDirectDrawSurface7.cpp
* Copyright © learn_more
*/
#pragma once
#include "../pch.h"
#include <ddraw.h>
#include <d3d11.h>

enum ETextureType
{
	TX_UNDEF,
	TX_LEAF,
	TX_WOOD,
};

class zCTexture;
class D3D11Texture;
class MyDirectDrawSurface7 : public IDirectDrawSurface7
{
public:
	MyDirectDrawSurface7();
	~MyDirectDrawSurface7();

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

	/** Binds this texture */
	void BindToSlot(int slot);

	/** Returns the engine texture of this surface */
	D3D11Texture* GetEngineTexture();
	
	/** Returns the normalmap of this surface */
	D3D11Texture* GetNormalmap();

	/** Returns the fx-map for this surface */
	D3D11Texture* GetFxMap();

	/** Loads additional resources if possible */
	void LoadAdditionalResources(zCTexture* ownedTexture);

	/** Returns the name of this surface */
	const std::string& GetTextureName();

	/** Sets this texture ready to use */
	void SetReady(bool ready){IsReady = ready;}

	/** returns if this surface is ready or not */
	bool IsSurfaceReady(){return IsReady;}

	/** Returns true if this surface is used to render a movie to */
	bool IsMovieSurface(){return LockedData != nullptr;}

	/** Adds one to the queued mipmap count */
	void IncreaseQueuedMipMapCount();

	/** Returns whether the mip-maps were put into the command queue or not */
	bool MipMapsInQueue();

	/** Returns the type of this texture */
	ETextureType GetTextureType(){ return TextureType;};
private:
	
	/** Faked attached surfaces for the mipmaps */
	std::vector<MyDirectDrawSurface7 *> attachedSurfaces;
	int refCount;

	/** Temporary data used during locks */
	unsigned char* LockedData;
	bool IsReady; // True if the attached texture was successfully filled with data

	/** Original DESC this was created with */
	DDSURFACEDESC2 OriginalSurfaceDesc;

	/** Attached texture */
	D3D11Texture* EngineTexture;

	/** Associated Name */
	std::string TextureName;
	ETextureType TextureType;

	/** Additional maps */
	D3D11Texture* Normalmap;
	D3D11Texture* FxMap;

	/** Locktype */
	DWORD LockType;

	/** zCTexture this is associated with */
	zCTexture* GothicTexture;

	/** Number of mipmaps in GPU queue */
	unsigned int QueuedMipMaps;
};
