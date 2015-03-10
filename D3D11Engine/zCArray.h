#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

template <class T> 
class zCArray {
public:
	~zCArray()
	{
		delete[] Array;
	}


	T* Array;
	int	NumAlloc;
	int	NumInArray;

	void AllocAbs(int num)
	{
		Array = new T[num];
		NumAlloc = num;
		NumInArray = num;
	}
};