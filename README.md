# GD3D11

With this mod, I want to bring the engine of Gothic 1&2 into a more modern state. Through an own implementation of the DirectDraw-API and using hooking and assemblercode-modifications of gothics intern engine calls, I have managed to completely replace gothics old rendering architecture with a new one, which is able to utilize more of the current GPU generations power for rendering.
Since Gothic in its original state tries to cull as much as possible, this takes a lot of work from the CPU, which was slowing down the game even on todays processors. While the original renderer did a really great job with the tech from 2002, GPUs have grown much faster.

And now, that they can actually use their power to render, we not only get a big performance boost on most systems, but also more features

To build the Mod, you need to have the following set up:

* Microsoft Visual Studio 2012 C++ (Express is fine!)
* DirectX SDK (June 2010)
* Environmentvariables "G2_SYSTEM_PATH" and/or "G1_SYSTEM_PATH", which should point to the "system"-folders of the games
* The latest Version of the Mod installed for your game

Since we can't use a debug-build, because mixing debug- and release-DLLs is not allowed, there are Release-Targets which turn optimization off so you are actually able to use the debugger from VisualStudio with the DLL. These targets will copy the dll, pdb and the shaders to the games system-folder.
