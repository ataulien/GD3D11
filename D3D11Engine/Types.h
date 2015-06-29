#pragma once
#include <string>
#include <Windows.h>
#include <D3DX10math.h>

/** Defines types used for the project */

/** Errorcodes */
enum XRESULT
{
	XR_SUCCESS,
	XR_FAILED,
	XR_INVALID_ARG,
};



struct INT2
{
	INT2(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	INT2(const D3DXVECTOR2& v)
	{
		this->x = (int)(v.x + 0.5f);
		this->y = (int)(v.y + 0.5f);
	}

	INT2(){}

	std::string toString()
	{
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}

	int x;
	int y;
};


struct INT4
{
	INT4(int x, int y, int z, int w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	INT4(){}

	int x;
	int y;
	int z;
	int w;
};


struct float4;
struct D3DXVECTOR3;
struct float3
{
	float3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	float3(const DWORD& color)
	{
		BYTE r = (color >> 16) & 0xFF;
		BYTE g = (color >> 8 ) & 0xFF;
		BYTE b = color & 0xFF;

		x = r / 255.0f;
		y = g / 255.0f;
		z = b / 255.0f;
	}

	float3(const D3DXVECTOR3& v)
	{
		x = ((float3 *)&v)->x;
		y = ((float3 *)&v)->y;
		z = ((float3 *)&v)->z;
	}

	float3(const float4& v)
	{
		x = ((float3 *)&v)->x;
		y = ((float3 *)&v)->y;
		z = ((float3 *)&v)->z;
	}

	D3DXVECTOR3* toD3DXVECTOR3() const
	{
		return (D3DXVECTOR3 *)this;
	}

	std::string toString() const
	{
		return std::string("(") + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
	}

	/** Checks if this float3 is in the range a of the given float3 */
	bool isLike(const float3& f, float a) const
	{
		float3 t;
		t.x = abs(x - f.x);
		t.y = abs(y - f.y);
		t.z = abs(z - f.z);

		return t.x < a && t.y < a && t.z < a;
	}

	static float3 FromColor(unsigned char r, unsigned char g, unsigned char b)
	{
		return float3(r / 255.0f, g / 255.0f, b / 255.0f);
	}

	bool operator < (const float3& rhs) const 
	{
		if ((z < rhs.z)) {return true;}
		if ((z == rhs.z) && (y < rhs.y)) {return true;}
		if ((z == rhs.z) && (y == rhs.y) && (x < rhs.x)) {return true;}
		return false;
	}

	bool operator == (const float3& b) const 
	{
		return isLike(b, 0.0001f);
	}

	float3(){}

	float x,y,z;
};

struct float4
{ 
	float4(const DWORD& color)
	{
		BYTE a = color >> 24;
		BYTE r = (color >> 16) & 0xFF;
		BYTE g = (color >> 8 ) & 0xFF;
		BYTE b = color & 0xFF;

		x = r / 255.0f;
		y = g / 255.0f;
		z = b / 255.0f;
		w = a / 255.0f;

	}

	float4(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	float4(const D3DXVECTOR4& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		this->w = v.w;
	}


	float4(const float3& f)
	{
		this->x = f.x;
		this->y = f.y;
		this->z = f.z;
		this->w = 1.0f;
	}

	float4(const D3DXVECTOR3& f)
	{
		this->x = f.x;
		this->y = f.y;
		this->z = f.z;
		this->w = 1.0f;
	}

	float4(const float3& f, float a)
	{
		this->x = f.x;
		this->y = f.y;
		this->z = f.z;
		this->w = a;
	}

	float4(){}

	D3DXVECTOR4* toD3DXVECTOR4() const
	{
		return (D3DXVECTOR4 *)this;
	}

	D3DXVECTOR3* toD3DXVECTOR3() const
	{
		return (D3DXVECTOR3 *)this;
	}


	DWORD ToDWORD() const
	{
		BYTE a = (BYTE)(w * 255.0f);
		BYTE r = (BYTE)(x * 255.0f);
		BYTE g = (BYTE)(y * 255.0f);
		BYTE b = (BYTE)(z * 255.0f);

		char c[4];
		c[0] = a;
		c[1] = r;
		c[2] = g;
		c[3] = b;

		return *(DWORD *)c;
	}

	float x,y,z,w;
};

struct float2
{
	float2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	float2(int x, int y)
	{
		this->x = (float)x;
		this->y = (float)y;
	}

	float2(const INT2& i)
	{
		this->x = (float)i.x;
		this->y = (float)i.y;
	}

	float2(const D3DXVECTOR2& v)
	{
		this->x = v.x;
		this->y = v.y;
	}

	float2(){}

	D3DXVECTOR2* toD3DXVECTOR2() const
	{
		return (D3DXVECTOR2 *)this;
	}

	std::string toString() const
	{
		return std::string("(") + std::to_string(x) + ", " + std::to_string(y) + ")";
	}

	bool operator < (const float2& rhs) const 
	{
		if ((y < rhs.y)) {return true;}
		if ((y == rhs.y) && (x < rhs.x)) {return true;}
		return false;
	}

	float x,y;
};