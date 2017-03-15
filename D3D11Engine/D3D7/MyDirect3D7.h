#pragma once
#include "../pch.h"
#include "MyDirect3DDevice7.h"
#include "MyDirect3DVertexBuffer7.h"
#include "../Engine.h"


class MyDirect3D7 : public IDirect3D7 {
public:
    MyDirect3D7(IDirect3D7* direct3d7) {
        DebugWrite("MyDirect3D7::MyDirect3D7\n");

		Direct3d7 = direct3d7;
		RefCount = 1;
    }

    /*** IUnknown methods ***/
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) {
        DebugWrite("MyDirect3D7::QueryInterface\n");
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() {
        DebugWrite("MyDirect3D7::AddRef\n");
		RefCount++;
        return RefCount;
    }

    ULONG STDMETHODCALLTYPE Release() {
        DebugWrite("MyDirect3D7::Release\n");
        RefCount--;
        if (0 == RefCount) {
            delete this;
			return 0;
        }

        return RefCount;
    }

    /*** IDirect3D7 methods ***/
    HRESULT STDMETHODCALLTYPE CreateDevice (REFCLSID rclsid, LPDIRECTDRAWSURFACE7 lpDDS, LPDIRECT3DDEVICE7* lplpD3DDevice) {
        DebugWrite("MyDirect3D7::CreateDevice\n");
        HRESULT hr = S_OK;

        if (SUCCEEDED(hr)) {
            *lplpD3DDevice = new MyDirect3DDevice7(this, nullptr);
        }

        return hr;
	}

	HRESULT STDMETHODCALLTYPE CreateVertexBuffer(LPD3DVERTEXBUFFERDESC lpVBDesc, LPDIRECT3DVERTEXBUFFER7* lplpD3DVertexBuffer, DWORD dwFlags) {
		DebugWrite("MyDirect3D7::CreateVertexBuffer\n");

		// Fake a vertexbuffer
		MyDirect3DVertexBuffer7* buff = new MyDirect3DVertexBuffer7(*lpVBDesc);
		*lplpD3DVertexBuffer = buff;

		return S_OK;
    }

	struct DeviceEnumInfo
	{
		LPD3DENUMDEVICESCALLBACK7 originalFn;
		LPVOID originalUserArg;
	};

	static HRESULT WINAPI DeviceEnumCallback(LPSTR strDesc,
                                  LPSTR strName, D3DDEVICEDESC7* pDesc,
                                  VOID* pParentInfo )
	{
		
		DeviceEnumInfo info = *(DeviceEnumInfo *)pParentInfo;
		LPD3DENUMDEVICESCALLBACK7 fn = (LPD3DENUMDEVICESCALLBACK7)info.originalFn;
		return (*fn)(strDesc, strName, pDesc, info.originalUserArg);
	}

    HRESULT STDMETHODCALLTYPE EnumDevices(LPD3DENUMDEVICESCALLBACK7 lpEnumDevicesCallback, LPVOID lpUserArg) {
        DebugWrite("MyDirect3D7::EnumDevices\n");

		// Open the device data gothic wants
		FILE* f = fopen("system\\GD3D11\\data\\DeviceEnum.bin", "rb");
		if(!f)
		{
			LogError() << "Failed to open the system\\GD3D11\\data\\DeviceEnum.bin file. Can't fake a device for Gothic now!";
			return E_FAIL;
		}

		char desc[256]; 
		char name[256]; 
		D3DDEVICEDESC7 devDesc;

		fread(desc, 256, 1, f);
		fread(name, 256, 1, f);
		fread(&devDesc, sizeof(D3DDEVICEDESC7), 1, f);

		fclose(f);

		LogInfo() << "Faking Device for Gothic: " << desc << " (" << name << ")";

		// And pass it to the callback function
		(*lpEnumDevicesCallback)(desc, name, &devDesc, lpUserArg);

		return S_OK;
        //return this->direct3d7->EnumDevices(lpEnumDevicesCallback, lpUserArg);
    }

    HRESULT STDMETHODCALLTYPE EvictManagedTextures() {
        DebugWrite("MyDirect3D7::EvictManagedTextures\n");
        return S_OK;
    }

	struct ZBufferEnumInfo
	{
		LPD3DENUMPIXELFORMATSCALLBACK originalFn;
		LPVOID originalUserArg;
	};

	static HRESULT CALLBACK D3DEnumPixelFormatsCallback(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
	{
		ZBufferEnumInfo info = *(ZBufferEnumInfo *)lpContext;
		
		/*FILE* f = fopen("system\\GD3D11\\data\\ZBufferEnum.bin", "ab");
		fwrite(lpDDPixFmt, sizeof(DDPIXELFORMAT), 1, f);
		fclose(f);*/

		return (*info.originalFn)(lpDDPixFmt, info.originalUserArg);
	}

	HRESULT STDMETHODCALLTYPE EnumZBufferFormats(REFCLSID riidDevice, LPD3DENUMPIXELFORMATSCALLBACK lpEnumCallback, LPVOID lpContext) {
		DebugWrite("MyDirect3D7::EnumZBufferFormats\n");

		/*ZBufferEnumInfo info;
		info.originalFn = lpEnumCallback;
		info.originalUserArg = lpContext;
		
        return this->direct3d7->EnumZBufferFormats(riidDevice, D3DEnumPixelFormatsCallback, &info);
		//return this->direct3d7->EnumZBufferFormats(riidDevice, lpEnumCallback, lpContext);*/

		DDPIXELFORMAT fmt;
		FILE* f = fopen("system\\GD3D11\\data\\ZBufferEnum.bin", "rb");

		LogInfo() << "Trying to find a working ZBuffer format for Gothic";
		while(!feof(f))
		{
			fread(&fmt, sizeof(DDPIXELFORMAT), 1, f);

			/*LogInfo() << "RefreshRate: " << desc.dwRefreshRate << "\n";
			LogInfo() << "Width: " << desc.dwWidth << "\n";
			LogInfo() << "Height: " << desc.dwHeight << "\n";
			LogInfo() << " --- ";*/

			(*lpEnumCallback)(&fmt, lpContext);
		}

		fclose(f);

		return S_OK;
    }

private:
    IDirect3D7* Direct3d7;
	int RefCount;
};

