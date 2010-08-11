#pragma once

#include "WinThread.h"

extern HINSTANCE hInstance;

void ErrorMsg(LPTSTR lpszFunction);

class HookThread: public WinThread
{
	public:
		HookThread();
	protected:
		void Setup();
		void Execute(void*);
};

// A monitor thread, brought to you by Razer! 
// (not really. Razer just got Blizzard to output some registry keys for them apparently. 
// Thanks, for making this DLL possible, guys (especially FakeSteve)!)
class RazerMonitorThread: public WinThread
{
	public:
		RazerMonitorThread();
	protected:
		void Setup();
		void Execute(void*);
};

class HotkeyThread: public WinThread
{
	public:
		HotkeyThread();
	protected:
		void Setup();
		void Execute(void*);
};

// Structure for storing all the many options for APMAlert2. There are a bunch :) :( :)
struct APMAlertOptions {
	// Alert
	DWORD		minAPM;
	DWORD		alertKey;
	char *		alertSound;
	bool		alertEnabled;

	// Clock
	D3DCOLOR	clockColor;
	D3DCOLOR	clockBgColor;
	char *		clockFontName;
	float		clockFontSize;
	bool		clockFontBold;
	bool		clockFontItalic;
	float		clockXPosition;
	float		clockYPosition;
	bool		clockCenterX;
	bool		clockCenterY;
	bool		clockEnabled;

	// LiveAPM
	D3DCOLOR	apmLabelColor;
	D3DCOLOR	apmBgColor;
	char *		apmFontName;
	float		apmFontSize;
	bool		apmFontBold;
	bool		apmFontItalic;
	float		apmXPosition;
	float		apmYPosition;
	bool		apmCenterX;
	bool		apmCenterY;
	DWORD		apmRangeBottom0;
	DWORD		apmRangeBottom1;
	DWORD		apmRangeBottom2;
	DWORD		apmRangeBottom3;
	DWORD		apmRangeBottom4;
	D3DCOLOR	apmRangeColor0;
	D3DCOLOR	apmRangeColor1;
	D3DCOLOR	apmRangeColor2;
	D3DCOLOR	apmRangeColor3;
	D3DCOLOR	apmRangeColor4;
	bool		apmEnabled;
};

// Functions we use that are part of external libraries
typedef IDirect3D9 * (__stdcall *pDirect3DCreate9)(UINT SDKVersion);
extern pDirect3DCreate9 D3DCreate;
typedef BOOL (__stdcall *pPlaySoundA)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
extern pPlaySoundA oPlaySound;
typedef HRESULT (__stdcall *pD3DXCreateFont)(__in LPDIRECT3DDEVICE9 pDevice, __in INT Height, __in UINT Width, __in UINT Weight,
								__in UINT MipLevels, __in BOOL Italic, __in DWORD CharSet, __in DWORD OutputPrecision,
								__in DWORD Quality, __in DWORD PitchAndFamily, __in LPCSTR pFacename, 
								__out LPD3DXFONT *ppFont);
extern pD3DXCreateFont oD3DXCreateFont;
typedef HRESULT (__stdcall *pD3DXCreateLine)(__in LPDIRECT3DDEVICE9 pDevice, __out LPD3DXLINE *ppLine);
extern pD3DXCreateLine oD3DXCreateLine;


extern ID3DXFont* pApmFont;
extern ID3DXFont* pClockFont;
extern ID3DXLine* pLine;

extern APMAlertOptions * apmOptions;
extern DWORD apm;
extern bool isIngame;
extern bool isMothershipBuilding;
extern bool isObs;

void DrawLine(int iStartX, int iStartY, int iEndX, int iEndY, int iWidth, D3DCOLOR d3dColor);
void FillArea(int iX, int iY, int iWidth, int iHeight, D3DCOLOR d3dColor);

void PreEndScene(IDirect3DDevice9 * ppD3DDevice);
void PostEndScene(IDirect3DDevice9 * ppD3DDevice);

void drawLiveApm(IDirect3DDevice9 * ppD3DDevice, IDirect3DSwapChain9 * pSwapChain, UINT width, UINT height);
void drawClock(IDirect3DDevice9 * ppD3DDevice, IDirect3DSwapChain9 * pSwapChain, UINT width, UINT height);

APMAlertOptions * LoadAPMAlertOptions();
void FreeAPMAlertOptions(APMAlertOptions * opts);

void initializeAPMCalculator();
void addAPM(DWORD newAPM);
void addAPM();

void toggleAlert();

void initializeLogger();
void writeToLog(char * str);
void logError(char * str);
void logInfo(char * str);
void freeLogger();