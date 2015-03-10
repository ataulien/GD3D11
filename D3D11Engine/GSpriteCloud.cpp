#include "pch.h"
#include "GSpriteCloud.h"


GSpriteCloud::GSpriteCloud(void)
{
}


GSpriteCloud::~GSpriteCloud(void)
{
}

struct CloudBB
{
	void MakeRandom(const D3DXVECTOR3& center, const D3DXVECTOR3& minSize, const D3DXVECTOR3& maxSize)
	{
		D3DXVECTOR3 d = maxSize - minSize;
		
		// Random Box size
		D3DXVECTOR3 sr = D3DXVECTOR3(minSize.x + Toolbox::frand() * d.x,
			minSize.y + Toolbox::frand() * d.y,
			minSize.z + Toolbox::frand() * d.z);
		sr /= 2.0f;
		Size = sr;

		Center = center;
	}

	D3DXVECTOR3 GetRandomPointInBox()
	{
		D3DXVECTOR3 r = D3DXVECTOR3((Toolbox::frand() * Size.x * 2) - Size.x,
			(Toolbox::frand() * Size.y * 2) - Size.y,
			(Toolbox::frand() * Size.z * 2) - Size.z);

		return r;
	}

	D3DXVECTOR3 Center;
	D3DXVECTOR3 Size;
};

/** Initializes this cloud */
void GSpriteCloud::CreateCloud(const D3DXVECTOR3& size, int numSprites)
{
	CloudBB c;
	c.MakeRandom(D3DXVECTOR3(0,0,0), size / 2.0f, size);

	// Fill the bb with sprites
	for(int i=0;i<numSprites;i++)
	{
		D3DXVECTOR3 rnd = c.GetRandomPointInBox();
		Sprites.push_back(rnd);

		D3DXMATRIX m;
		D3DXMatrixTranslation(&m, rnd.x, rnd.y, rnd.z);

		SpriteWorldMatrices.push_back(m);
	}
}