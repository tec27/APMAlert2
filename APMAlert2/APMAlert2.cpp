#include "stdafx.h"
#include "APMAlert2.h"
#include <io.h>
#include <fcntl.h>
#include <strsafe.h>
#include <mmsystem.h>
#include <DxErr.h>

#pragma comment(lib, "dxerr.lib")

// Function pointers to replace linking in the libs
pDirect3DCreate9 D3DCreate;
pPlaySoundA oPlaySound;
pD3DXCreateFont oD3DXCreateFont;
pD3DXCreateLine oD3DXCreateLine;

void * addrEndScene;

unsigned char backup_EndScene[6];
unsigned char patch_EndScene[6];

bool hooksSetup = false;
HINSTANCE hInstance;
APMAlertOptions * apmOptions = NULL;

DWORD lastAlertTick = 0;
bool minPassed = false;

void ErrorMsg(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    //MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("APMAlert2 Error"), MB_OK);
	logError((char*)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

void WritePatch(void * addr, unsigned char * patch) {
	DWORD oldProtect;
	if(VirtualProtect(addr, 6, PAGE_EXECUTE_READWRITE, &oldProtect) == FALSE) return;
	if(IsBadWritePtr(addr, 6)) return;
	memcpy(addr, (void *)patch, 6);
	if(VirtualProtect(addr, 6, oldProtect, &oldProtect) == FALSE) return;
}

HRESULT __stdcall hkEndScene(IDirect3DDevice9 * ppD3DDevice)
{
	// hook for IDirect3DDevice9::EndScene()
	PreEndScene(ppD3DDevice);
	WritePatch(addrEndScene, backup_EndScene); // restore the function to how it is normally

	HRESULT retVal = ppD3DDevice->EndScene(); // call the function to complete the game's request and store its return value
	
	WritePatch(addrEndScene, patch_EndScene); // restore our hook patch to maintain the hook
	PostEndScene(ppD3DDevice);
	
	return retVal; // return what the actual function call returned so we stay "invisible"
}

void SetupHooks() {
	logInfo("Setting up hooks.");
	if(hooksSetup) return;

	HMODULE d3dMod = GetModuleHandle("d3d9.dll");
	if(d3dMod == NULL) {
		ErrorMsg("GetModuleHandle(d3d9.dll)");
		return;
	}
	HMODULE d3dxMod = LoadLibrary("d3dx9_43.dll");
	if(d3dxMod == NULL) {
		ErrorMsg("LoadLibrary(d3dx9_43.dll)");
		return;
	}
	HMODULE winmmMod = LoadLibrary("winmm.dll");
	if(winmmMod == NULL) {
		ErrorMsg("LoadLibrary(winmm.dll)");
		return;
	}

	D3DCreate = (pDirect3DCreate9)GetProcAddress(d3dMod, "Direct3DCreate9");
	if(D3DCreate == NULL) {
		ErrorMsg("GetProcAddress(d3dMod, \"Direct3DCreate9\")");
		return;
	}
	oPlaySound = (pPlaySoundA)GetProcAddress(winmmMod, "PlaySoundA");
	if(oPlaySound == NULL) {
		ErrorMsg("GetProcAddress(winmmMod, \"PlaySoundA\")");
		return;
	}
	oD3DXCreateFont = (pD3DXCreateFont)GetProcAddress(d3dxMod, "D3DXCreateFontA");
	if(oD3DXCreateFont == NULL) {
		ErrorMsg("GetProcAddress(d3dxMod, \"D3DXCreateFontA\")");
		return;
	}
	oD3DXCreateLine = (pD3DXCreateLine)GetProcAddress(d3dxMod, "D3DXCreateLine");
	if(oD3DXCreateLine == NULL) {
		ErrorMsg("GetProcAddress(d3dxMod, \"D3DXCreateLine\")");
		return;
	}

	// Create a dummy window to call CreateDevice on
    HWND hwnd;
	hwnd = CreateWindow("BUTTON", "APMAlertDummyWindow", 0, 0, 0, 27, 27, NULL, NULL, hInstance, NULL);
	if(hwnd == NULL) {
		ErrorMsg("CreateWindow");
		return;
	}

    //UpdateWindow(hwnd);

	IDirect3D9 *pD3D = D3DCreate(D3D_SDK_VERSION);
	if(pD3D == NULL) {
		ErrorMsg("Direct3DCreate9");
		return;
	}
	D3DDISPLAYMODE d3ddm;
	HRESULT hRes = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if(FAILED(hRes)) { 
		char errorMsg[512];
		const char * dxErrorStr = DXGetErrorString(hRes);
		sprintf_s(errorMsg, 512, "GetAdapterDisplayMode returned 0x%08x: %s", hRes, dxErrorStr);
		logError(errorMsg);
		goto cleanup;
	}
	D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = true;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = d3ddm.Format;

	IDirect3DDevice9 * ppD3DDevice;

	hRes = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
						&d3dpp, &ppD3DDevice);
	if(FAILED(hRes)) {
		char errorMsg[512];
		const char * dxErrorStr = DXGetErrorString(hRes);
		sprintf_s(errorMsg, 512, "CreateDevice returned 0x%08x: %s", hRes, dxErrorStr);
		logError(errorMsg);
		goto cleanup;
	}

	// Get our function pointers from the virtual table
	// This pointer dereferencing works because the virtual table is the first item in memory
	//  of the every object
	void ** vTable = *((void***)ppD3DDevice);

	// Access the function pointers we need
	addrEndScene = vTable[42]; // EndScene is the 43rd function (you can just count in the interface decl in the header)

	/*
	char path_d3d9_dll[MAX_PATH];
	GetSystemDirectory(path_d3d9_dll, MAX_PATH);	
	strncat_s(path_d3d9_dll, MAX_PATH, "\\d3d9.dll", 10);
	SIZE_T offset = (unsigned int)addrEndScene - (unsigned int)GetModuleHandle(path_d3d9_dll);
	printf("EndScene() Addr: 0x%08x -- SC2.exe!d3d9.dll+0x%x\n", addrEndScene, offset);
	*/

	DWORD oldProtect;
	// backup the top 6 bytes of each function
	if(VirtualProtect(addrEndScene, 6, PAGE_EXECUTE_READWRITE, &oldProtect) == FALSE) {
		ErrorMsg("VirtualProtect");
		return; // make the address read/writable
	}
	memcpy(backup_EndScene, addrEndScene, 6);
	VirtualProtect(addrEndScene, 6, oldProtect, &oldProtect); // restore old protection

	// We are going to write over the top 6 bytes of every function we want to hook.
	// This way, whenever they are called, we can jump to our custom hook function and run our own stuff.
	// To maintain proper game functionality we will restore the backup code, run the function as it should be, 
	// then restore our patch code at the top when it returns to our hook function.
	
	// create our 6 byte patch consisting of: push <addr_hook>; retn (essentially a call that doesn't disturb the stack)
	patch_EndScene[0] = 0x68; // PUSH
	*((DWORD *)(patch_EndScene+1)) = (DWORD)&hkEndScene; // value to push
	patch_EndScene[5] = 0xC3; // RETN

	hooksSetup = true;

	logInfo("Hooks setup and ready for use.");

	ppD3DDevice->Release();
	ppD3DDevice = NULL;
	pD3D->Release();
	pD3D = NULL;

cleanup:
	if(pD3D != NULL)
		pD3D->Release();
	// Destroy the dummy window
	DestroyWindow(hwnd);
}

void WriteHooks() {
	if(!hooksSetup) return;
	WritePatch(addrEndScene, patch_EndScene);
	logInfo("Hooks installed.");
}

void ClearHooks() {
	if(!hooksSetup) return;
	WritePatch(addrEndScene, backup_EndScene);
	logInfo("Hooks cleared.");
}

void doAlert() {
	if(!minPassed && apm > (apmOptions->minAPM * 1.1f))
		minPassed = true;

	if(!isObs && apmOptions->alertEnabled && apm < apmOptions->minAPM && minPassed) {
		DWORD curTick = GetTickCount();
		if(curTick - lastAlertTick > 2000) {
			oPlaySound(apmOptions->alertSound, NULL, SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
			lastAlertTick = curTick;
		}
	}
}

void toggleAlert() {
	apmOptions->alertEnabled = !apmOptions->alertEnabled;
	HKEY hKey;
	LONG lError;
	lError = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\tec27\\APMAlert2", 0, KEY_WRITE, &hKey);
	if(lError != ERROR_SUCCESS)
		return;

	DWORD data = apmOptions->alertEnabled;
	RegSetValueEx(hKey, "AlertEnabled", 0, REG_DWORD, (BYTE *)&data, sizeof(DWORD));
}

HookThread::HookThread() {}

void HookThread::Setup() {
	initializeLogger();
	logInfo("APMAlert2 starting up...");
}

void HookThread::Execute(void * data) {
	apmOptions = LoadAPMAlertOptions();

	RazerMonitorThread * rmt = new RazerMonitorThread();
	HotkeyThread * hkt = new HotkeyThread();
	rmt->Start(NULL);
	hkt->Start(NULL);

	bool wasIngame = false;

	while(!isTerminated()) {
		if(!hooksSetup) {
			SetupHooks();
			if(!hooksSetup)
				Sleep(5000);
		}
		else {
			if(isIngame && !wasIngame) {
				logInfo("Game detected. Beginning initialization.");
				isObs = true;
				if(apmOptions != NULL)
					FreeAPMAlertOptions(apmOptions);
				apmOptions = LoadAPMAlertOptions();
				if(pApmFont != NULL) pApmFont->Release();
				if(pClockFont != NULL) pClockFont->Release();
				pApmFont = NULL;
				pClockFont = NULL;
				initializeAPMCalculator();
				minPassed = false;
				WriteHooks();
				wasIngame = isIngame;
			}
			else if(wasIngame && !isIngame) {
				logInfo("Game end detected.");
				ClearHooks();
				wasIngame = isIngame;
			}

			if(isIngame) {
				doAlert();
			}
		}
		Sleep(100);
	}
	logInfo("APMAlert2 beginning termination.");
	rmt->setTerminated(true);
	hkt->setTerminated(true);

	ClearHooks();
	logInfo("Freeing resources.");
	if(pApmFont != NULL) {
		pApmFont->Release();
		pApmFont = NULL;
	}
	if(pClockFont != NULL) {
		pClockFont->Release();
		pClockFont = NULL;
	}
	if(pLine != NULL) {
		pLine->Release();
		pLine = NULL;
	}
	if(apmOptions != NULL)
		FreeAPMAlertOptions(apmOptions);
	logInfo("Termination complete.");
	freeLogger();
}