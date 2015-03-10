#pragma once
#include "pch.h"

class GSpriteCloud
{
public:
	GSpriteCloud(void);
	~GSpriteCloud(void);

	/** Initializes this cloud */
	void CreateCloud(const D3DXVECTOR3& size, int numSprites = 10);

	/** Returns the sprite world matrices */
	const std::vector<D3DXMATRIX>& GetWorldMatrices();

protected:
	/** World matrices for the sprites */
	std::vector<D3DXVECTOR3> Sprites;

	/** Sprite positions as world matrices */
	std::vector<D3DXMATRIX> SpriteWorldMatrices;
};

