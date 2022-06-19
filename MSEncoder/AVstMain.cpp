//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Example AGain (VST 2.0)
// Stereo plugin which applies a Gain [-oo, 0dB]
// � 2003, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include "AVst.hpp"

bool oome = false;

#if MAC
#pragma export on
#endif

//------------------------------------------------------------------------
// Prototype of the export function main
//------------------------------------------------------------------------
/*
#if BEOS
#define main main_plugin
extern "C" __declspec(dllexport) AEffect *main_plugin (audioMasterCallback audioMaster);

#elif MACX
#define main main_macho
extern "C" AEffect *main_macho (audioMasterCallback audioMaster);

#else
AEffect *main (audioMasterCallback audioMaster);
#endif
*/

#if __GNUC__ && (WIN32 || BEOS)
extern "C" __declspec(dllexport)
AEffect* main_plugin (audioMasterCallback audioMaster) asm ("main");
#define main main_plugin

#else
AEffect *main (audioMasterCallback audioMaster);
#endif

//------------------------------------------------------------------------
AEffect *main (audioMasterCallback audioMaster)
{
	// Get VST Version
	if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version

	// Create the AudioEffect
	AVst* effect = new AVst (audioMaster);
	if (!effect)
		return 0;

	// Check if no problem in constructor of AGain
	if (oome)
	{
		delete effect;
		return 0;
	}
	return effect->getAeffect ();
}

#if MAC
#pragma export off
#endif

//------------------------------------------------------------------------
#if WIN32
#include <windows.h>
void* hInstance;
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;
	return 1;
}
#endif
