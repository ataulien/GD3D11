#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

template <class T>
class zCArrayAdapt
{
public:
	unsigned int GetSizeInBytes()
	{
		return NumInArray * sizeof(T);
	}

	T Get(unsigned int idx)
	{
		return Array[idx];
	}

	T* Array;
	int	NumInArray;
};