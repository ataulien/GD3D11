#pragma once
#include "pch.h"
#include "GothicMemoryLocations.h"

class zSTRING
{
public:

	zSTRING(const char* str)
	{
		XCALL(GothicMemoryLocations::zSTRING::ConstructorCharPtr);
	}


	const char* ToChar() const
	{
		XCALL(GothicMemoryLocations::zSTRING::ToChar);
	}

	char data[20];
};