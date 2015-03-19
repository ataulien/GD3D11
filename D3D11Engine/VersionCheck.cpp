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
#endif

	}
}