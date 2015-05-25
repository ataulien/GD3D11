#pragma once
#include "pch.h"
#include <thread>

// zipname, userdata, unpackedfiles, numfiles
typedef void (__cdecl* UnzipDoneCallback)(const std::string&, void*, int,int);

class ZipArchive
{
public:
	ZipArchive(void);
	~ZipArchive(void);

	/** unzips the given archive */
	static XRESULT Unzip(const std::string& zip, const std::string& target);
	XRESULT UnzipThreaded(const std::string& zip, const std::string& target, UnzipDoneCallback callback, void* cbUserdata);

private:

	static void UnzipThreadFunc(const std::string& zip, const std::string& target, UnzipDoneCallback callback, void* cbUserdata);

	std::thread* UnzipThread;
};

