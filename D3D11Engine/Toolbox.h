#pragma once
#include <Windows.h>
#include <D3DX10.h>
#include <string>
#include "Types.h"

/** Misc. tools */

namespace Toolbox
{
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