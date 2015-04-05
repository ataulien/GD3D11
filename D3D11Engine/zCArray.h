#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

/*template <class T> 
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
};*/



/*////////////////////////////////////////////////////////////////////////////

This file is part of the G2Ext SDK headers.

//////////////////////////////////////////////////////////////////////////////

The G2Ext SDK headers

Copyright © 2009, 2010 by Paindevs and Patrick Vogel

All Rights reserved.

THE WORK (AS DEFINED BELOW) IS PROVIDED
UNDER THE TERMS OF THIS CREATIVE COMMONS
PUBLIC LICENSE ("CCPL" OR "LICENSE").
THE WORK IS PROTECTED BY COPYRIGHT AND/OR
OTHER APPLICABLE LAW. ANY USE OF THE WORK
OTHER THAN AS AUTHORIZED UNDER THIS LICENSE
OR COPYRIGHT LAW IS PROHIBITED.

BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED
HERE, YOU ACCEPT AND AGREE TO BE BOUND BY THE
TERMS OF THIS LICENSE. TO THE EXTENT THIS
LICENSE MAY BE CONSIDERED TO BE A CONTRACT,
THE LICENSOR GRANTS YOU THE RIGHTS CONTAINED
HERE IN CONSIDERATION OF YOUR ACCEPTANCE OF
SUCH TERMS AND CONDITIONS.

Full license at http://creativecommons.org/licenses/by-nc/3.0/legalcode

/////////////////////////////////////////////////////////////////////////////*/


/** Array class wrapper
* @attention: The classes zCList, zCArray, zCListSort, zCArraySort, zCTree, etc. are not intended for regular usage. Please use the STL equivalents for your coding!
*/

template<class T>
class zCArray
{
public:
        T*  Array;
        int NumAlloc;
        int NumInArray;

private:
        void Reallocate(const int nSize)
        {
                if(nSize == 0)
                        return;

                T* pArray = new T[this->NumAlloc+nSize];

                if(this->NumInArray > 0)
                {
                        for(int i = 0; i < this->NumInArray; i++)
                                pArray[i] = array[i];
                };

                delete [] this->Array;
                this->NumAlloc += nSize;
                this->Array     = pArray;
        };      

public:
        /** Insert description.
        */
        zCArray()
        {
                this->Array           = NULL;
                this->NumAlloc        = 0;
                this->NumInArray      = 0;
        };

        /** Insert description.
        */
        ~zCArray()
        {
                this->Clear();
        };

        /** Insert description.
        */
        const T& operator [] (const unsigned int pos) const
        {
                if((int)pos <= this->NumInArray)
                        return this->Array[pos];
        };

        /** Insert description.
        */
        T& operator [] (const unsigned int pos)
        {
                if((int)pos <= this->NumInArray)
                        return this->Array[pos];
        };

        /** Insert description.
        */
        void Clear()
        {
                this->NumAlloc        = 0;
                this->NumInArray      = 0;
                if(this->Array != NULL)
                {
                        delete[] this->Array;
                        this->Array = NULL;
                };
        };

        /**
        * @brief Adds an element to the zCArray.
        *
        * This method adds an element to the zCArray. If the preallocated memory of the zCArray
        * is full then this method will allocate one additional field to store the the new element.
        * This behaviour saves memory but is very inefficient when adding many elements as memory
        * is allocated on every method call when the preallocated area is full.
        *
        * This method can be interleaved with PushBackFast.
        */
        void PushBack(const T& in)
        {
                if((this->NumInArray + 1) > this->NumAlloc) // -- PB throws around with memory. But we don't, so we use the allocated space and don't allocate more and more memory... (memory leak!?)
                        Reallocate(1);

                this->Array[this->NumInArray++] = in;
        };

        /**
        * @brief Adds an element to the zCArray.
        *
        * This method adds an element to the zCArray. If the preallocated memory of the zCArray
        * is full then this method will double the size of the preallocated memory. This might lead
        * to the worst case situation where twice as much memory than needed is used for this zCArray.
        * Use this method in often created/short lived zCArrays.
        *
        * This method can be interleaved with PushBack.
        */
        void PushBackFast(const T& in)
        {
                if((this->NumInArray + 1) > this->NumAlloc) // -- Let's throw around with memory like PB does for performance's sake.
                        Reallocate(this->NumAlloc*2);

                this->Array[this->NumInArray++] = in;
        };

        /** Insert description.
        */
        const T& GetItem(const unsigned int pos)
        {
                if((pos <= this->NumInArray) && (pos <= this->NumAlloc))
                        return this->Array[pos];
        };

        /** Insert description.
        */
        unsigned int GetSize(void)
        {
                return (unsigned int)this->NumInArray;
        };

        /** Insert description.
        */
        int Search(const T& item)
        {
                for (size_t i = 0; i < GetSize(); i++)
                {
                        if (Array[i] == item)
                                return i;
                }

                return -1;
        };

        /** Insert description.
        */
        bool IsInList(const T& item)
        {
                for (size_t i = 0; i < GetSize(); i++)
                {
                        if (Array[i] == item)
                                return true;
                }

                return false;
        };
};