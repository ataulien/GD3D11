#include "VersionCheck.h"
#include <Windows.h>
#include <ImageHlp.h>
#include <string>
#include "Logger.h"

namespace VersionCheck
{
	static const int CHECKSUM_G2_2_6_FIX = 0x008a3e89;
	static const int CHECKSUM_G2_1_08k = 0x0000eb3d;

	/** Returns whether the given file exists */
	bool FileExists(const std::string& file)
	{
		FILE* f = fopen(file.c_str(), "rb");

		if(f)
		{
			fclose(f);
			return true;
		}
		return false;
	}

	/** Checks the executable checksum for the right version */
	void CheckExecutable()
	{
		// Get checksum from header
		DWORD checksum;
		DWORD headersum;

		char dir[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, dir);

		std::string exe = dir + std::string("\\Gothic.exe");

		if(!FileExists(exe))
		{
			exe = dir + std::string("\\Gothic2.exe");
			if(!FileExists(exe))
			{
				LogWarnBox() << "Failed to find the Game-Executable! Continuing without version-check...";
				return;
			}
		}

		// Get checksum from header
		MapFileAndCheckSum(exe.c_str(), &headersum, &checksum);

#ifdef BUILD_GOTHIC_2_6_fix
		if(headersum != CHECKSUM_G2_2_6_FIX)
		{
			LogWarnBox() << "Your Gothic-Executable does not match the checksum for this Version of GD3D11!\n"
				"This DLL only works for Gothic 2 - The Night Of The Raven, Version 2.6 (fix) or the System-Pack.\n\n"
				"You can continue and try anyways but the game will most likely crash.\n";
		}
#endif

#ifdef BUILD_GOTHIC_1_08k
		if(headersum != CHECKSUM_G2_1_08k)
		{
			LogWarnBox() << "Your Gothic-Executable does not match the checksum for this Version of GD3D11!\n"
				"This DLL only works for Gothic 1 - Version 1.08k_mod or the System-Pack.\n\n"
				"You can continue and try anyways but the game will most likely crash.\n";
		}

		LogInfoBox() << "You are using the Gothic 1 Version of GD3D11. This is not an official release, so please keep that in mind!\n"
						"Not everything is working yet and it may crash frequently. You don't need to report every bug you see, because I likely have seen it myself by now.\n";

		if(Toolbox::FileExists("Systempack.ini"))
		{
			LogInfoBox() << "Systempack detected. With this installed, menus won't show up. GD3D11 will still work, but you will have to navigate the menus blindly.\n"
							"This will be fixed in a future release\n";
		}
#endif

		// Check for game data
		// Do this by checking one file in the folder which should always be there
		// Not the best solution, but we'll just roll with it at this point as
		// this is only a hint for the user that he forgot to copy over the GD3D11-Folder
		if(!Toolbox::FileExists("GD3D11\\data\\DeviceEnum.bin"))
		{
			LogErrorBox() << "Failed to find GD3D11 systemfiles!\n"
							 "This means: The GD3D11-folder is missing or corrupt. This can be the result of only copying the ddraw.dll into Gothics system-folder, which isn't enough!\n\n"
							 "Please check your installation.\n";
			exit(0);
		}
	}
}