#include "pch.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "D3D11GraphicsEngineTest.h"
//#include "ReferenceD3D11GraphicsEngine.h"
#include "D3D11AntTweakBar.h"
#include "HookExceptionFilter.h"

namespace Engine
{
	const std::string DLL_FILES[] = { "d3dcompiler_46.dll", "FW1FontWrapper.dll", "Assimp32.dll", "AntTweakBar.dll" };
	const int NUM_DLL_FILES = 4;

	/** Creates main graphics engine */
	void CreateGraphicsEngine()
	{
		LogInfo() << "Creating Main graphics engine";
		GraphicsEngine = new D3D11GraphicsEngine;

		if(!GraphicsEngine)
		{
			LogErrorBox() << "Failed to create GraphicsEngine! Out of memory!";
			exit(0);
		}

		XLE(GraphicsEngine->Init());

		// Create ant tweak bar with it
		AntTweakBar = new D3D11AntTweakBar;
	}

	/** Creates the Global GAPI-Object */
	void CreateGothicAPI()
	{
		LogInfo() << "GD3D11 " << VERSION_STRING;

		LogInfo() << "Loading modules for stacktracer";
		MyStackWalker::GetSingleton(); // Inits the static object in there

		LogInfo() << "Initializing GothicAPI";

		GAPI = new GothicAPI;
		if(!GAPI)
		{
			LogErrorBox() << "Failed to create GothicAPI! Out of memory!";
			exit(0);
		}
	}

	/** Loads the needed dll files from subdir */
	void LoadDLLFiles()
	{
		volatile int* i = new int;
		i = 0;

		// Load dll files from subdir
		for(int i=0;i<NUM_DLL_FILES;i++)
		{
			LoadLibrary((ENGINE_BASE_DIR + DLL_FILES[i]).c_str());
		}
	}

	/** Called when the game is about to close */
	void OnShutDown()
	{
		LogInfo() << "Shutting down...";

		delete Engine::AntTweakBar;	Engine::AntTweakBar = NULL;
		delete Engine::GAPI; Engine::GAPI = NULL;
		delete Engine::GraphicsEngine; Engine::GraphicsEngine = NULL;
	}

};

