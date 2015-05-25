#pragma once
#include "d2ddialog.h"
#include "FileDownloader.h"
#include <mutex>

enum EDownloadType
{
	EDL_Normalmaps_Find,
};

struct DownloadJob
{
	DownloadJob(){}

	DownloadJob(const std::string& package, const std::string& targetPath)
	{
		DownloadPackage = package;
		TargetPath = targetPath;
	}

	std::string DownloadPackage; // Package on the server
	std::string TargetPath; // Path to extract to
};

class SV_Label;
class SV_ProgressBar;
class FileDownloader;
class ZipArchive;
class D2DContentDownloadDialog :
	public D2DDialog
{
public:
	D2DContentDownloadDialog(D2DView* view, D2DSubView* parent, EDownloadType type, const std::list<DownloadJob>& jobs = std::list<DownloadJob>());
	~D2DContentDownloadDialog(void);

	/** Initializes the controls of this view */
	virtual XRESULT InitControls();

	/** Draws this sub-view */
	virtual void Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime);

	/** Starts the next job in the list */
	virtual void RunNextJob();
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
	DownloadJob Job;

	/** Downloader */
	FileDownloader* Downloader;

	/** ZipArchive */
	ZipArchive* Zip;
	int UnzipFile;
	int UnzipNumFiles;

	/** Mutex for updating the text */
	std::mutex DLMutex;

	/** List of next jobs */
	std::list<DownloadJob> NextJobs;
};

