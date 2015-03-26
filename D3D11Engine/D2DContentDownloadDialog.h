#pragma once
#include "d2ddialog.h"
#include "FileDownloader.h"
#include <mutex>

enum EDownloadType
{
	EDL_Normalmaps_Original,
};

class SV_Label;
class SV_ProgressBar;
class FileDownloader;
class ZipArchive;
class D2DContentDownloadDialog :
	public D2DDialog
{
public:
	D2DContentDownloadDialog(D2DView* view, D2DSubView* parent, EDownloadType type);
	~D2DContentDownloadDialog(void);

	/** Initializes the controls of this view */
	virtual XRESULT InitControls();

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

protected:

	/** Called whenever the download did something */
	static void ProgressCallback(FileDownloader* downloader, void* userdata, DownloadInfo info);

	static void UnzipDoneCallback(const std::string& zipname, void* userdata, int file, int numFiles);

	/** Text-Label */
	SV_Label* Message;
	SV_ProgressBar* ProgressBar;

	/** Type of download */
	EDownloadType DownloadType;
	std::string TargetFolder;

	/** Downloader */
	FileDownloader* Downloader;

	/** ZipArchive */
	ZipArchive* Zip;
	int UnzipFile;
	int UnzipNumFiles;

	/** Mutex for updating the text */
	std::mutex DLMutex;
};

