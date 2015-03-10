#pragma once

template <class T> 
class zCTree 
{
public:
	zCTree* Parent;
	zCTree* FirstChild;
	zCTree* Next;
	zCTree* Prev;
	T* Data;
};