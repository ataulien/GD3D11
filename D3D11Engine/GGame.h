#pragma once

/** Thin wrapper of everything the game calls and needs to run */

class zCBspTree;
class GWorld;
class zCVob;
class zCWorld;
class GGame
{
public:
	GGame(void);
	virtual ~GGame(void);

	/** Called on render */
	virtual void DrawWorld();

	/** Called when a vob got added to a world */
	virtual void OnAddVob(zCVob* vob, zCWorld* world);

	/** Called when a vob got removed from a world */
	virtual void OnRemoveVob(zCVob* vob, zCWorld* world);

	/** Called when a vob moved */
	virtual void OnVobMoved(zCVob* vob);

	/** Called on a SetVisual-Call of a vob */
	void OnSetVisual(zCVob* vob);

	/** Called when the game is done loading the world */
	virtual void OnWorldLoaded();

	/** Creates a new world-object, discards the old one */
	void SwitchActiveWorld();

	/** Called when the geometry of a world was successfully loaded */
	void OnGeometryLoaded(zCBspTree* bspTree);

	/** Returns the currently active world */
	GWorld* GetWorld();
protected:

	/** The currently active world */
	GWorld* ActiveWorld;

	std::future<void> RenderFuture;
};

