#pragma once
#include "zSTRING.h"

class zCObject
{
public:
	int vtbl;

	int refCtr;
	VERTEX_INDEX hashIndex;
	zCObject* hashNext;
	zSTRING objectName;	
};

