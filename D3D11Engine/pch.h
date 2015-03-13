#pragma once

/** Precompiled header */
#pragma message ("Creating precompiled header data")

#define NOMINMAX

#pragma warning(disable: 4731) // Change of ebp from inline assembly

#include <d3d11.h>
#include "Types.h"
#include "Logger.h"
#include "VertexTypes.h"
#include <map>
#include <unordered_map>
#include <hash_map>
#include <hash_set>
#include <list>

#include <set>
#include <signal.h>

const char* const VERSION_NUMBER = "12";
const char* const VERSION_STRING = "Version X12";

/** D3D7-Call logging */
#define DebugWriteValue(value, check) if(value == check){LogInfo() << " - " << #check;}
#define DebugWriteFlag(value, check) if((value & check) == check){LogInfo() << " - " << #check;}
#define DebugWrite(debugMessage) DebugWrite_i(debugMessage, (void*)this);

/** Debugging */
#define SAFE_RELEASE(x) if(x){x->Release(); x=NULL;}
#define SAFE_DELETE(x) delete x; x=NULL;
//#define V(x) x

/** Writes a string of the D3D7-Call log */
void DebugWrite_i(LPCSTR lpDebugMessage, void* thisptr);

/** Computes the size in bytes of the given FVF */
int ComputeFVFSize( DWORD fvf );