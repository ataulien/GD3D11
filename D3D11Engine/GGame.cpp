#include "pch.h"
#include "GGame.h"
#include "GWorld.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"

GGame::GGame(void)
{
	ActiveWorld = NULL;
}


GGame::~GGame(void)
{
	delete ActiveWorld;
}

/** Called when the game is done loading the world */
void GGame::OnWorldLoaded()
{
	// Traverse the games BSP-Tree and collect all the vobs
	ActiveWorld->BuildBSPTree(Engine::GAPI->GetLoadedWorldInfo()->BspTree);
}

/** Called when a vob got added to a world */
void GGame::OnAddVob(zCVob* vob, zCWorld* world)
{
	// Create a new world if we currently have none
	if(!ActiveWorld)
		SwitchActiveWorld();

	ActiveWorld->AddVob(vob, world);
}

/** Called when a vob got removed from a world */
void GGame::OnRemoveVob(zCVob* vob, zCWorld* world)
{
	// Create a new world if we currently have none
	if(!ActiveWorld)
		SwitchActiveWorld();

	ActiveWorld->RemoveVob(vob, world);
}

/** Called when a vob moved */
void GGame::OnVobMoved(zCVob* vob)
{
	// Create a new world if we currently have none
	if(!ActiveWorld)
		SwitchActiveWorld();

	ActiveWorld->AddVob(vob, vob->GetHomeWorld(), true);
}

/** Called on a SetVisual-Call of a vob */
void GGame::OnSetVisual(zCVob* vob)
{
	// Re-Add the vob // TODO: We only need to change the visual, but this is easier. Don't be lazy!
	if(ActiveWorld->RemoveVob(vob, vob->GetHomeWorld()))
		ActiveWorld->AddVob(vob, vob->GetHomeWorld(), true);
}

/** Called when the geometry of a world was successfully loaded */
void GGame::OnGeometryLoaded(zCBspTree* bspTree)
{
	// Create a new world if we currently have none
	if(!ActiveWorld)
		SwitchActiveWorld();

	// Tell the world to extract the world-mesh
	ActiveWorld->ExtractWorldMesh(bspTree);
}

/** Creates a new world-object, discards the old one */
void GGame::SwitchActiveWorld()
{
	delete ActiveWorld;
	ActiveWorld = new GWorld;
}

/** Called on render */
void GGame::DrawWorld()
{
	if(ActiveWorld && !Engine::GAPI->GetRendererState()->RendererSettings.DisableRendering)
	{
		ActiveWorld->DrawWorld();
	}
}

/** Returns the currently active world */
GWorld* GGame::GetWorld()
{
	return ActiveWorld;
}