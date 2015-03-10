#pragma once
#include "pch.h"
#include "zCWorld.h"
#include "WorldConverter.h"

class GInventory
{
public:
	GInventory(void);
	~GInventory(void);

	/** Called when a VOB got added to the BSP-Tree or the world */
	void OnAddVob(VobInfo* vob, zCWorld* world);

	/** Called when a VOB got removed from the world */
	bool OnRemovedVob(zCVob* vob, zCWorld* world);

	/** Draws the inventory for the given world */
	void DrawInventory(zCWorld* world, zCCamera& camera);

private:
	std::map<zCWorld *, std::list<VobInfo *>> InventoryVobs;
};

