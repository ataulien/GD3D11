#include "pch.h"
#include "D2DContentDownloadDialog.h"
#include "D2DSubView.h"
#include "D2DView.h"
#include "SV_Label.h"
#include "SV_Panel.h"
#include "SV_ProgressBar.h"
#include "FileDownloader.h"
#include "ZipArchive.h"
#include "ModSpecific.h"

/** TODO: This file has hardcoded URLs, create a config or something else for this! */

D2DContentDownloadDialog::D2DContentDownloadDialog(D2DView* view, D2DSubView* parent, EDownloadType type, const std::list<DownloadJob>& jobs ) : D2DDialog(view, parent)
{
	Downloader = NULL;
	Zip = NULL;

	NextJobs = jobs;

	DownloadType = type;
	UnzipNumFiles = 0;
	UnzipFile = 0;
	InitControls();
	RunNextJob();
}


D2DContentDownloadDialog::~D2DContentDownloadDialog(void)
{
	delete Downloader;
	delete Zip;
}

/** Initializes the controls of this view */
XRESULT D2DContentDownloadDialog::InitControls()
{
	D2DSubView::InitControls();
	
	// Position in top left corner
	SetPositionAndSize(D2D1::Point2F(10, 10), D2D1::SizeF(400, 55));

	// Create message label
	/*Message = new SV_Label(MainView, MainPanel);
	Message->SetSize(GetSize());
	Message->AlignUnder(Header, 5.0f);
	Message->SetPosition(D2D1::Point2F(5, Message->GetPosition().y));

	Message->SetCaption("Text");*/

	ProgressBar = new SV_ProgressBar(MainView, MainPanel);
	ProgressBar->SetPositionAndSize(D2D1::Point2F(5,25 + 5), D2D1::SizeF(GetSize().width - 10, 20));
	ProgressBar->SetProgress(0.0f);

	return XR_SUCCESS;
}

/** Called whenever the download did something */
void D2DContentDownloadDialog::ProgressCallback(FileDownloader* downloader, void* userdata, DownloadInfo info)
{
	D2DContentDownloadDialog* v = (D2DContentDownloadDialog *)userdata;

	//if(info.szStatusText)
	//	v->Message->SetCaption(Toolbox::ToMultiByte(info.szStatusText));

	v->ProgressBar->SetProgress((float)info.ulProgress / (float)info.ulProgressMax);

	// If this is true, we are at the last line of the download-thread
	if(downloader->GetProgress().isDone)
	{
		// Start the unzipping
		v->Zip->UnzipThreaded(info.targetFile, v->TargetFolder, D2DContentDownloadDialog::UnzipDoneCallback, v);
	}
}

void D2DContentDownloadDialog::UnzipDoneCallback(const std::string& zipname, void* userdata, int file, int numFiles)
{
	D2DContentDownloadDialog* v = (D2DContentDownloadDialog *)userdata;

	v->UnzipFile = file;
	v->UnzipNumFiles = numFiles;

	v->ProgressBar->SetProgress((float)file / (float)numFiles);

}

/** Starts the next job in the list */
void D2DContentDownloadDialog::RunNextJob()
{
	// Clean up potential done job
	delete Zip; Zip = NULL;
	delete Downloader; Downloader = NULL;

	if(NextJobs.empty())
	{
		SetHidden(true); // FIXME: Add method to actually close these dialogs...

		// We're done with everything, apply changes
		Engine::GAPI->ReloadTextures();

		return;
	}

	Zip = new ZipArchive;

	// Get next job
	Job = NextJobs.front();
	NextJobs.pop_front();

	LogInfo() << "Downloading file: " << Job.DownloadPackage;

	// Start new download
	switch(DownloadType)
	{
	case EDL_Normalmaps_Find:
	default:
		{
			// Find which normalmap package we have to download
			std::string pck = ModSpecific::GetModNormalmapPackName();

			Header->SetCaption("Downloading...");
			CreateDirectory(Job.TargetPath.c_str(), NULL);
			TargetFolder = Job.TargetPath.c_str();
		}
		break;
	}

	try{
		Downloader = new FileDownloader("http://www.gothic-dx11.de/download/" + Job.DownloadPackage, Job.DownloadPackage, ProgressCallback, this);
	}catch(std::exception e)
	{
		LogErrorBox() << "Failed to start content download. Reason: " << e.what();
		delete Downloader;

		// FIXME: Ugly. Should close this on error.
		SetHidden(true); 
		return;
	}

	
}

/** Draws this sub-view */
void D2DContentDownloadDialog::Draw(const D2D1_RECT_F& clientRectAbs, float deltaTime)
{
	std::string message;

	if(!Downloader)
		return;

	// FIXME: This should be in an update-function
	// Start the next job if we are done unzipping or the download failed
	if(UnzipFile == UnzipNumFiles - 1 || Downloader->GetProgress().hasFailed)
	{
		UnzipFile = 0;
		UnzipNumFiles = 0;

		// Done!
		// Check for other jobs
		RunNextJob();
	}

	// Need to check again if the download failed and "RunNextJob()" has deleted the downloader
	if(!Downloader || IsHidden())
		return;

	// Size in mbyte
	float dlCurrent = Downloader->GetProgress().LastInfo.ulProgress / (1024.0f * 1024.0f);
	float dlMax = Downloader->GetProgress().LastInfo.ulProgressMax / (1024.0f * 1024.0f);

	if(dlCurrent != dlMax &&
		Downloader->GetProgress().LastInfo.ulProgressMax != 0)
	{
		switch(Downloader->GetProgress().LastInfo.ulStatusCode)
		{
		case BINDSTATUS_FINDINGRESOURCE:
			message = "Finding resource...";
			break;
		case BINDSTATUS_CONNECTING:
			message = "Connecting...";
			break;
		case BINDSTATUS_SENDINGREQUEST:
			message = "Sending request...";
			break;
		case BINDSTATUS_MIMETYPEAVAILABLE:
			message = "Mime type available";
			break;
		case BINDSTATUS_CACHEFILENAMEAVAILABLE:
			message = "Cache filename available";
			break;
		case BINDSTATUS_BEGINDOWNLOADDATA:
			message = "Begin download";
			break;

		default:
			{
				char txt[256];
				sprintf_s(txt, "Downloading %s ... (%.2f/%.2f MBytes)", Job.DownloadPackage.c_str(), dlCurrent, dlMax);
				message = txt;
			}
		}
	}
	
	if(UnzipNumFiles > 0) // Unzip-Phase
	{
		char txt[256];
		sprintf_s(txt, "Unpacking... (%d/%d files)", UnzipFile, UnzipNumFiles);
		message = txt;

		if(UnzipFile == UnzipNumFiles - 1)
		{
			SetHidden(true);
			MainView->AddMessageBox("Success!", "Successfully downloaded and unpacked data. Please restart the game to apply the changes!");
		}
	}

	Header->SetCaption(message);

	D2DDialog::Draw(clientRectAbs, deltaTime);
	
}