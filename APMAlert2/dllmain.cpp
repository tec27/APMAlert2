// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "APMAlert2.h"

HookThread * hookThread = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInstance = hModule;
		hookThread = new HookThread();
		hookThread->Start(NULL);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		hookThread->setTerminated(true);
		break;
	}
	return TRUE;
}

