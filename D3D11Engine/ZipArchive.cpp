#include "pch.h"
#include "ZipArchive.h"
#include "XUnzip.h"

ZipArchive::ZipArchive(void)
{
	UnzipThread = NULL;
}


ZipArchive::~ZipArchive(void)
{
	if(UnzipThread)
	{
		UnzipThread->join(); // Wait for the thread to complete
		delete UnzipThread;
	}
}

/** unzips the given archive */
XRESULT ZipArchive::Unzip(const std::string& zip, const std::string& target)
{
	LogInfo() << "Unzipping file: " << zip;

	HZIP hz = OpenZip((void *)zip.c_str(), 0, ZIP_FILENAME);

	if(!hz)
	{
		LogWarn() << "Failed to open Zip-File: " << zip;
		return XR_FAILED;
	}

	// Get last entry
	ZIPENTRY ze;
	ZRESULT zr = GetZipItem(hz, -1, &ze); 

	if (zr == ZR_OK)
	{
		int numItems = ze.index;

		LogInfo() << " - Found " << numItems << " items in ZipFile";

		for(int i=0;i<numItems;i++)
		{
			GetZipItem(hz, i, &ze); 
			std::string t = target + "\\" + ze.name;
			LogInfo() << " - Unzipping file to: " << t;;

			UnzipItem(hz, i, (void *)t.c_str(), 0, ZIP_FILENAME);
		}
	}
	
	CloseZip(hz);

	return XR_SUCCESS;
}

XRESULT ZipArchive::UnzipThreaded(const std::string& zip, const std::string& target, UnzipDoneCallback callback, void* cbUserdata)
{
	if(UnzipThread)
		return XR_FAILED; // Already doing something

	// Start unzip thread
	delete UnzipThread;
	UnzipThread = new std::thread(UnzipThreadFunc, zip, target, callback, cbUserdata);

	return XR_SUCCESS;
}

void ZipArchive::UnzipThreadFunc(const std::string& zip, const std::string& target, UnzipDoneCallback callback, void* cbUserdata)
{
	LogInfo() << "Unzipping file: " << zip;

	HZIP hz = OpenZip((void *)zip.c_str(), 0, ZIP_FILENAME);

	if(!hz)
	{
		LogWarn() << "Failed to open Zip-File: " << zip;
		return;
	}

	// Get last entry
	ZIPENTRY ze;
	ZRESULT zr = GetZipItem(hz, -1, &ze); 

	if (zr == ZR_OK)
	{
		int numItems = ze.index;

		LogInfo() << " - Found " << numItems << " items in ZipFile";

		for(int i=0;i<numItems;i++)
		{
			GetZipItem(hz, i, &ze); 
			std::string t = target + "\\" + ze.name;
			LogInfo() << " - Unzipping file to: " << t;;

			UnzipItem(hz, i, (void *)t.c_str(), 0, ZIP_FILENAME);

			callback(zip, cbUserdata, i, numItems);
		}
	}
	
	CloseZip(hz);
}