#pragma once
#include "pch.h"

/** Global engine information */
class BaseGraphicsEngine;
class GothicAPI;
class BaseAntTweakBar;
class GGame;
class ThreadPool;

__declspec( selectany ) const char* ENGINE_BASE_DIR = "system\\GD3D11\\";

#if _MSC_VER < 1900
__declspec( selectany ) const char* VERSION_STRING = "Version X" VERSION_NUMBER " ("__DATE__")";
#else
__declspec(selectany) const char* VERSION_STRING = "Version X" VERSION_NUMBER;
#endif

namespace Engine
{
	struct DLLInfo
	{
		HANDLE AntTweakBar;
		HANDLE Assimp32;
	};

	/** If true, we will just pass everything to the usual ddraw.dll */
	__declspec( selectany ) bool PassThrough;

	/** Global engine object */
	__declspec( selectany ) BaseGraphicsEngine* GraphicsEngine;

	/** Global GothicAPI object */
	__declspec( selectany ) GothicAPI* GAPI;

	/** Global AntTweakBar object */
	__declspec( selectany ) BaseAntTweakBar* AntTweakBar;

	/** Global rendering threadpool */
	__declspec(selectany) ThreadPool* RenderingThreadPool;

	/** Global worker threadpool */
	__declspec(selectany) ThreadPool* WorkerThreadPool;

	/** Creates main graphics engine */
	void CreateGraphicsEngine();

	/** Creates the Global GAPI-Object */
	void CreateGothicAPI();

	/** Loads the needed dll files from subdir */
	void LoadDLLFiles();

	/** Called when the game is about to close */
	void OnShutDown();
};

