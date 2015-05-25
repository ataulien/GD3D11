#include "pch.h"
#include "FileDownloader.h"
#include <thread>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")

HRESULT __stdcall DownloadProgress::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
	//wcout << ulProgress << L" of " << ulProgressMax;
	//if (szStatusText) wcout << " " << szStatusText;
	//wcout << endl;

	switch(ulStatusCode)
	{
	case BINDSTATUS_FINDINGRESOURCE:
		LogInfo() << "Finding resource...";
		break;
	case BINDSTATUS_CONNECTING:
		LogInfo() << "Connecting...";
		break;
	case BINDSTATUS_SENDINGREQUEST:
		LogInfo() << "Sending request...";
		break;
	case BINDSTATUS_MIMETYPEAVAILABLE:
		LogInfo() << "Mime type available";
		break;
	case BINDSTATUS_CACHEFILENAMEAVAILABLE:
		LogInfo() << "Cache filename available";
		break;
	case BINDSTATUS_BEGINDOWNLOADDATA:
		LogInfo() << "Begin download";
		break;
	case BINDSTATUS_DOWNLOADINGDATA:
	case BINDSTATUS_ENDDOWNLOADDATA:
		{
			int percent = (int)(100.0 * static_cast<double>(ulProgress)
				/ static_cast<double>(ulProgressMax));
			
			LogInfo() << "Percent: " << percent;
		}
		break;

	default:
		{
			LogInfo() << "Status code: " << ulStatusCode;
		}
	}

	DownloadInfo info;
	info.ulProgress = ulProgress;
	info.ulProgressMax = ulProgressMax;
	info.ulStatusCode = ulStatusCode;
	info.szStatusText = szStatusText;
	info.targetFile = targetFile;

	LastInfo = info;

	OwningDownloader->CallProgressCallback(info);

	//isDone = ulProgress == ulProgressMax;
	return S_OK;
}

FileDownloader::FileDownloader(const std::string& url, const std::string& targetFile, DownloadProgressCallback progressCallback, void* callbackUserdata)
{
	ProgressCallback = progressCallback;
	CallbackUserdata = callbackUserdata;

	Progress.OwningDownloader = this;
	Progress.targetFile = targetFile;
	Progress.isDone = false;
	Progress.hasFailed = false;

	// Create worker thread and start download
	DownloadThread = std::thread(DownloadThreadFunc, this, url, targetFile, progressCallback, callbackUserdata);
}


FileDownloader::~FileDownloader(void)
{
	DownloadThread.join();
}

/** Calls the progress callback */
void FileDownloader::CallProgressCallback(DownloadInfo info)
{
	ProgressCallback(this, CallbackUserdata, info);
}

void FileDownloader::DownloadThreadFunc(FileDownloader* dl, const std::string& url, const std::string& targetFile, DownloadProgressCallback progressCallback, void* callbackUserdata)
{
	// Makke sure we re-download it
	DeleteUrlCacheEntry(url.c_str());

	// Start download
	HRESULT hr = URLDownloadToFile(NULL, url.c_str(), targetFile.c_str(), 0, &dl->Progress);

	if(FAILED(hr))
	{
		LogWarn() << "Failed to start contentdownload of " << url << " to " << targetFile;
		dl->Progress.hasFailed = true;
		return;
	}


	DownloadInfo info;
	info.ulProgress = 0;
	info.ulProgressMax = 0;
	info.ulStatusCode = 0;
	info.szStatusText = 0;
	info.targetFile = targetFile;

	dl->Progress.isDone = true;
	progressCallback(dl, dl->CallbackUserdata, info);
}