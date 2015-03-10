#pragma once
#include "pch.h"
#include "GothicMemoryLocations.h"

class zSTRING
{
public:
	const char* ToChar() const
	{
		XCALL(GothicMemoryLocations::zSTRING::ToChar);
	};

	char data[20];
};