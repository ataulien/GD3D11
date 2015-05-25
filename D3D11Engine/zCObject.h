#pragma once
#include "zSTRING.h"

class zCClassDef;
class zCObject
{
public:
	void Release()
	{
		refCtr--;
		int temp = refCtr;
		if(temp <= 0)
		{
			// Call destructor
			delete this;
		}
	}

	// Recreate V-Table
	virtual	zCClassDef*	_GetClassDef() = 0;
	virtual void Archive() = 0;
	virtual void Unarchive() = 0;
	virtual ~zCObject(){}
	/*void Destructor(char arg)
	{
		// Get vtable-entry
		int* vtbl = (int*)((int*)this)[0];

		typedef void(__thiscall* pFun)(void*, char);

		pFun fn = (pFun)vtbl[GothicMemoryLocations::zCObject::VTBL_ScalarDestructor];
		fn(this, arg);
	}*/

	int refCtr;
	VERTEX_INDEX hashIndex;
	zCObject* hashNext;
	zSTRING objectName;	
};

