#pragma once
#include "pch.h"

/** List of mods which supply a normalmap-package */ // TODO: Don't hardcode this!
enum ELoadedMod
{
	LM_Original = 0,
	LM_Vurt = 1,
	LM_LHiver = 3,
	LM_Odyssee = 4
};



namespace ModSpecific
{
	__declspec(selectany) const char* NRMPACK_ORIGINAL = "Normalmaps_Original";
	__declspec(selectany) const char* NRMPACK_VURT = "Normalmaps_Vurt";
	__declspec(selectany) const char* NRMPACK_LHIVER = "Normalmaps_LHiver";
	__declspec(selectany) const char* NRMPACK_ODYSSEY = "Normalmaps_Odyssey";

	/** Returns the normalmap-package for the currently played mod */
	std::string GetModNormalmapPackName();

	/** Checks whether the given normalmap-package is installed or not */
	bool NormalmapPackageInstalled(const std::string& package);
};