#pragma once
#include <thread>
#include "pch.h"

struct DownloadInfo
{
	ULONG ulProgress;
	ULONG ulProgressMax;
	ULONG ulStatusCode;
	LPCWSTR szStatusText;
	std::string targetFile;
};

class FileDownloader;
typedef void (__cdecl* DownloadProgressCallback)(FileDownloader*, void*, DownloadInfo);

class DownloadProgress : public IBindStatusCallback {
public:
	HRESULT __stdcall QueryInterface(const IID &,void **) { 
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef(void) { 
		return 1;
	}
	ULONG STDMETHODCALLTYPE Release(void) {
		return 1;
	}
	HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, IBinding *pib) {

		lastProgress = 0;
		lastProgressMax = 0;
		isDone = false;
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE GetPriority(LONG *pnPriority) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved) {
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, LPCWSTR szError) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo) {
		return E_NOTIMPL;
	}
	virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed) {
		return E_NOTIMPL;
	}        
	virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID riid, IUnknown *punk) {
		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText);


	bool isDone;
	ULONG lastProgress;
	ULONG lastProgressMax;
	std::string targetFile;
	DownloadInfo LastInfo;
	FileDownloader* OwningDownloader;
};

class FileDownloader
{
public:
	FileDownloader(const std::string& url, const std::string& targetFile, DownloadProgressCallback progressCallback, void* callbackUserdata);
	~FileDownloader(void);

	/** Calls the progress callback */
	void CallProgressCallback(DownloadInfo info);

	/** Returns the files current progress */
	DownloadProgress& GetProgress(){return Progress;}

protected:

	static void DownloadThreadFunc(FileDownloader* dl, const std::string& url, const std::string& targetFile, DownloadProgressCallback progressCallback, void* callbackUserdata);

	DownloadProgressCallback ProgressCallback;
	void* CallbackUserdata;
	DownloadProgress Progress;

	std::thread DownloadThread;
};

