#include "pch.h"
#include "ModSpecific.h"

namespace ModSpecific
{
	/** Returns the normalmap-package for the currently played mod */
	std::string GetModNormalmapPackName()
	{
		std::string gameini = zCOption::GetOptions()->ParameterValue("Game"); // Get the value of the "game"-commandline parameter

		if(gameini == "GOTHICGAME.INI")
			return NRMPACK_ORIGINAL;

		// This hopefully gets all versions of this mod
		if(gameini.find("HIVER") != std::string::npos)
			return NRMPACK_LHIVER;

		if(gameini == "ODYSSEY.INI")
			return NRMPACK_ODYSSEY;

		return NRMPACK_ORIGINAL;
	}

	/** Checks whether the given normalmap-package is installed or not */
	bool NormalmapPackageInstalled(const std::string& package)
	{
		std::string path = "system\\GD3D11\\Textures\\Replacements\\" + package;

		if(Toolbox::FolderExists(path))
			return true; // Folder is there, so assume it has been filled with files (It's not bad when it wasn't)

		return false;
	}
};