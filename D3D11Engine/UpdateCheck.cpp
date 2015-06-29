#include "pch.h"
#include "UpdateCheck.h"

#pragma comment(lib, "urlmon.lib")

// TODO: Make a config file for this!
const char* UPDATE_URL = "http://www.gothic-dx11.de/download/updates/g2d3d11.txt";
const char* UPDATER_EXE = "G2D3D11Updater.exe";
const char* UPDATER_PATH = "system\\GD3D11\\Data\\G2D3D11Updater.exe";

/*class DownloadProgress : public IBindStatusCallback {
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

	virtual HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
	{
		//wcout << ulProgress << L" of " << ulProgressMax;
		//if (szStatusText) wcout << " " << szStatusText;
		//wcout << endl;

		isDone = ulProgress == ulProgressMax;
		return S_OK;
	}

	bool isDone;
};*/


UpdateCheck::UpdateCheck(void)
{
}


UpdateCheck::~UpdateCheck(void)
{
}

/** Checks for update, returns new version URL if found */
std::string UpdateCheck::CheckForUpdate()
{
	/*DownloadProgress progress;
	progress.isDone = false;
	HRESULT hr = URLDownloadToFile(NULL, UPDATE_URL.c_str(), "Versions.txt", 0, &progress);

	if(hr == S_OK)
	{
		// Wait a second for the download to finish
		Sleep(1000);

		// If this did not succeed
		if(!progress.isDone)
		{
			// give it another second
			Sleep(1000);

			if(!progress.isDone)
			{
				LogWarn() << "Failed to search for updates!";
			}
		}
	}

	// Open the (hopefully) downloaded file
	FILE* f = fopen("Versions.txt", "r");
	if(!f)
		return "";

	while(!feof(f))
	{
		char txt[256];
		fgets(txt, 255, f);

		char versionNum[64];
		char zip[64];
		char changes[64];
		sscanf(txt, "%s;%s;%s", versionNum, zip, changes);

		

		LogInfo() << versionNum << " " << zip << " " << changes;
	}

	fclose(f);*/
}

void deleteDirectory(const std::string &path) 
{
	char buf[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buf);

	// Make sure we are really in the right directory
	if(!Toolbox::FileExists("system\\Gothic2.exe") && 
	   !Toolbox::FileExists("system\\Gothic 2.exe") &&
	   !Toolbox::FileExists("system\\Gothic.exe"))
	{
		return; // "Scary"!
	}


	system(("rm /s /q " + path).c_str());
}

/** Runs the updater and waits for it to finish */
void UpdateCheck::RunUpdater()
{
	// Check for really old version
	if(Toolbox::FileExists("System\\G2D3D11Launcher.exe"))
	{
		switch(MessageBoxA(NULL, "GD3D11 found a very old version in your systemfolder.\nSince it is not needed anymore, do you want to delete those files?\n"
			"This includes:\n"
			" - G2D3D11Launcher.exe\n"
			" - G2D3D11Launcher.exe.config\n"
			" - G2D3D11Updater.exe\n"
			" - data\\*\n"
			" - textures\\*\n"
			" - shaders\\*\n"
			" - shaderReplacements\\*\n"
			" - meshes\\*\n", "Very old version found!", MB_ICONASTERISK | MB_YESNO))
		{
		case IDYES:
			system("del system\\G2D3D11Launcher.exe");
			system("del system\\G2D3D11Launcher.exe.config");
			system("del system\\G2D3D11Updater.exe");
			
			deleteDirectory("system\\data");
			deleteDirectory("system\\textures");
			deleteDirectory("system\\shaders");
			deleteDirectory("system\\shaderReplacements");
			deleteDirectory("system\\meshes");

			MessageBox(NULL, "Done! There may be more files/folders you want to delete from the old version! You can ignore them, though, as they won't interfere with this version.", "More files?", MB_OK | MB_ICONINFORMATION);
			break;
		}
	}
	
	LogInfo() << "Running updater...";

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	memset(&startupInfo, 0, sizeof(startupInfo));
	memset(&processInfo, 0, sizeof(processInfo));
    startupInfo.cb = sizeof(startupInfo);          

	std::string cmdline = std::string(UPDATER_EXE) + " " + std::string(VERSION_NUMBER) + " " + std::to_string(GetCurrentProcessId());
	char c[64];
	strcpy_s(c, cmdline.c_str());

	/* Create the process */
	if (!CreateProcess(UPDATER_PATH,
					   c, NULL, NULL,
					   NULL, CREATE_DEFAULT_ERROR_MODE, NULL, NULL,
					   &startupInfo,
					   &processInfo
					  )
	   ) {
		printf("Failed to run Updater!: %d\n", GetLastError());
		return;
	}

	LogInfo() << "Waiting for updater...";

	// if an update was found, the updater will kill this process
	WaitForSingleObject(processInfo.hProcess, INFINITE);
}