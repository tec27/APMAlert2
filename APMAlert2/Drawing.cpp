#include "stdafx.h"
#include "APMAlert2.h"
#include <strsafe.h>
#include <DxErr.h>

ID3DXFont* pApmFont = NULL;
ID3DXFont* pClockFont = NULL;
ID3DXLine* pLine = NULL;

void CreateResourcesD3D9(IDirect3DDevice9* ppD3DDevice) {
	logInfo("Creating Direct3D resources.");
	HRESULT hr = oD3DXCreateFont(ppD3DDevice, 
							(int)apmOptions->apmFontSize,	// Height
							 0,								// Width (0 default)
							 (apmOptions->apmFontBold ? FW_BOLD : FW_NORMAL), // Weight
							 1,								// MipLevels
							 apmOptions->apmFontItalic,		// Italic
							 DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 ANTIALIASED_QUALITY,
							 DEFAULT_PITCH,
							 apmOptions->apmFontName,
							 &pApmFont);
	if(SUCCEEDED(hr)) {
		logInfo("Font for LiveAPM created successfully.");

		hr = oD3DXCreateLine(ppD3DDevice, &pLine);
		if(!SUCCEEDED(hr)) {
			char errorMsg[512];
			const char * dxErrorStr = DXGetErrorString(hr);
			sprintf_s(errorMsg, 512, "D3DXCreateLine returned 0x%08x: %s", hr, dxErrorStr);
			logError(errorMsg);
			pApmFont->Release();
			pApmFont = NULL;
			return;
		}
		logInfo("Line for drawing box backgrounds created successfully.");

		hr = oD3DXCreateFont(ppD3DDevice, 
							(int)apmOptions->clockFontSize,	// Height
							 0,								// Width (0 default)
							 (apmOptions->clockFontBold ? FW_BOLD : FW_NORMAL), // Weight
							 1,								// MipLevels
							 apmOptions->clockFontItalic,	// Italic
							 DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 ANTIALIASED_QUALITY,
							 DEFAULT_PITCH,
							 apmOptions->clockFontName,
							 &pClockFont);
		if(!SUCCEEDED(hr)) 
		{
			char errorMsg[512];
			const char * dxErrorStr = DXGetErrorString(hr);
			sprintf_s(errorMsg, 512, "D3DXCreateFont returned 0x%08x: %s", hr, dxErrorStr);
			logError(errorMsg);
			pApmFont->Release();
			pApmFont = NULL;
			pLine->Release();
			pLine = NULL;
			return;
		}

		logInfo("Font for Clock Display created successfully.");
	}
	else {
		char errorMsg[512];
		const char * dxErrorStr = DXGetErrorString(hr);
		sprintf_s(errorMsg, 512, "D3DXCreateFont returned 0x%08x: %s", hr, dxErrorStr);
		logError(errorMsg);
	}
	logInfo("All resources created successfully.");
}

void DrawLine(int iStartX, int iStartY, int iEndX, int iEndY, int iWidth, D3DCOLOR d3dColor) {
	pLine->SetWidth((float)iWidth);

	D3DXVECTOR2 d3dxVector[2];
	
	d3dxVector[0] = D3DXVECTOR2((float)iStartX, (float)iStartY);
	d3dxVector[1] = D3DXVECTOR2((float)iEndX, (float)iEndY);
	
	pLine->Begin();
	pLine->Draw(d3dxVector, 2, d3dColor);
	pLine->End();
}

void FillArea(int iX, int iY, int iWidth, int iHeight, D3DCOLOR d3dColor) {
	DrawLine(iX + iWidth / 2, iY, iX + iWidth / 2, iY + iHeight, iWidth, d3dColor);
}

void PreEndScene(IDirect3DDevice9 * ppD3DDevice) {
	IDirect3DSwapChain9 * pSwapChain;
	if(FAILED(ppD3DDevice->GetSwapChain(0, &pSwapChain)))
		return;

	D3DPRESENT_PARAMETERS pPresentationParams;
	if(FAILED(pSwapChain->GetPresentParameters(&pPresentationParams)))
		return;

	UINT height = pPresentationParams.BackBufferHeight;
	UINT width = pPresentationParams.BackBufferWidth;
	
	// Set the Viewport back to 0,0 and Width/Height of the device so that 
	//  we don't bug out when tooltips get rendered
	D3DVIEWPORT9 oldVp;
	ppD3DDevice->GetViewport(&oldVp);

	D3DVIEWPORT9 vp;
	vp.X = 0; vp.Y = 0;
	vp.Width = width; vp.Height = height;
	vp.MaxZ = 1.0f; vp.MinZ = 0.0f;
	ppD3DDevice->SetViewport(&vp);

	if(pApmFont == NULL || pClockFont == NULL || pLine == NULL)
		CreateResourcesD3D9(ppD3DDevice);

	drawLiveApm(ppD3DDevice, pSwapChain, width, height);
	drawClock(ppD3DDevice, pSwapChain, width, height);

	// Restore old viewport for compatibility
	ppD3DDevice->SetViewport(&oldVp);
	pSwapChain->Release();
}

void PostEndScene(IDirect3DDevice9 * ppD3DDevice) {}