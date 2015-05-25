#pragma once
#include <Windows.h>
#include <D3DX10.h>
#include <string>
#include <map>
#include <hash_map>
#include "Types.h"

/** Misc. tools */

namespace Toolbox
{
		/** Erases an element by value from a vector */
	template<typename T> void EraseByElement(std::vector<T>& vector, T value)
	{
		auto it = std::find(vector.begin(), vector.end(), value);

		if(it != vector.end())
			vector.erase(it);
	}

	/** Erases an element by value from a vector */
	template<typename T, typename S> void EraseByElement(std::map<T, S>& map, S value)
	{
		for(auto it = map.begin();it!=map.end();it++)
		{
			if((*it).second == value)
				it = map.erase(it);
		}
	}

	/** Deletes all elements of the given std::vector */
	template<typename T> void DeleteElements(std::vector<T>& vector)
	{
		for(auto it = vector.begin(); it != vector.end(); it++)
		{
			delete (*it);
		}

		vector.clear();
	}

	/** Deletes all elements of the given std::vector */
	template<typename T> void DeleteElements(std::list<T>& list)
	{
		for(auto it = list.begin(); it != list.end(); it++)
		{
			delete (*it);
		}

		list.clear();
	}

	/** Deletes all (second) elements of the given std::map */
	template<typename T, typename S> void DeleteElements(std::map<T, S>& map)
	{
		for(auto it = map.begin(); it != map.end(); it++)
		{
			delete (*it).second;
		}

		map.clear();
	}

	/** Deletes all (second) elements of the given std::hash_map */
	template<typename T, typename S> void DeleteElements(std::hash_map<T, S>& map)
	{
		for(auto it = map.begin(); it != map.end(); it++)
		{
			delete (*it).second;
		}

		map.clear();
	}

	/** Checks if a folder exists */
	bool FolderExists(const std::string& dirName_in);

	/** Hashes the given float value */
	void hash_combine(std::size_t& seed, float value);

	/** Hashes the given DWORD value */
	void hash_combine(std::size_t& seed, DWORD value);

	/** Returns true if the given position is inside the box */
	bool PositionInsideBox(const D3DXVECTOR3& p, const D3DXVECTOR3& min, const D3DXVECTOR3& max);

	/** Converts an errorcode into a string */
	std::string MakeErrorString(XRESULT code);

	/** Returns the number of bits inside a bitmask */
	WORD GetNumberOfBits( DWORD dwMask );

	/** Returns the size of a DDS-Image in bytes */
	unsigned int GetDDSStorageRequirements(unsigned int width, unsigned int height, bool dxt1);

	/** Returns the RowPitch-Size of a DDS-Image */
	unsigned int GetDDSRowPitchSize(unsigned int width, bool dxt1);

	/** Returns a random number between 0 and 1 */
	float frand();

	/** Linear interpolation */
	float lerp(float a, float b, float w);

	/** Converts a multi-byte-string to wide-char */
	std::wstring ToWideChar(const std::string& str);
	
	/** Converts a wide-char-string to  multi-byte*/
	std::string ToMultiByte(const std::wstring& str);

	/** Returns whether two AABBs are intersecting or not */
	bool AABBsOverlapping(const D3DXVECTOR3& minA, const D3DXVECTOR3& maxA, const D3DXVECTOR3& minB, const D3DXVECTOR3& maxB); 

	/** Does a ray vs aabb test */
	bool IntersectBox(const D3DXVECTOR3& min, const D3DXVECTOR3& max, const D3DXVECTOR3& origin, const D3DXVECTOR3& direction, float& t);

	/** Does a ray vs aabb test */
	bool IntersectTri(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2, const D3DXVECTOR3& origin, const D3DXVECTOR3& direction, float& u, float& v, float& t);

	/** Computes the normal of a triangle */
	D3DXVECTOR3 ComputeNormal(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, const D3DXVECTOR3& v2);

	/** Computes the distance of a point to an AABB */
	float ComputePointAABBDistance(const D3DXVECTOR3& p, const D3DXVECTOR3& min, const D3DXVECTOR3& max);

	/** Returns whether the given file exists */
	bool FileExists(const std::string& file);

	/** Saves a std::string to a FILE* */
	void SaveStringToFILE(FILE* f, const std::string& str);

	/** sse2 memcpy implementation by William Chan and Google */
	void X_aligned_memcpy_sse2(void* dest, const void* src, const unsigned long size_t);

	/** Loads a std::string from a FILE* */
	std::string LoadStringFromFILE(FILE* f);
};