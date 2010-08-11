#include "stdafx.h"
#include "APMAlert2.h"
#include <strsafe.h>

#define UPDATE_DELTA 500 // How many milliseconds between updating the time info
DWORD lastUpdate = 0;

const char * apmLabel = "APM: ";
const char * apmValueFormat = "%5d";

// Cached info
int xPos = 0;
int yPos = 0;
int borderSpacing = 0;
int boxWidth = 0;
int boxHeight = 0;
char apmStr[32];
D3DCOLOR curValueColor;
RECT labelRect, valueRect;

void drawLiveApm(IDirect3DDevice9 * ppD3DDevice, IDirect3DSwapChain9 * pSwapChain, UINT width, UINT height) {
	if(pApmFont != NULL && pLine != NULL && !isObs && apmOptions->apmEnabled) {
		DWORD curTick = GetTickCount();
		if(curTick - lastUpdate > UPDATE_DELTA) {
			sprintf_s(apmStr, 32, apmValueFormat, apm);

			curValueColor = apmOptions->apmRangeColor0;
			if(apm > apmOptions->apmRangeBottom1) curValueColor = apmOptions->apmRangeColor1;
			if(apm > apmOptions->apmRangeBottom2) curValueColor = apmOptions->apmRangeColor2;
			if(apm > apmOptions->apmRangeBottom3) curValueColor = apmOptions->apmRangeColor3;
			if(apm > apmOptions->apmRangeBottom4) curValueColor = apmOptions->apmRangeColor4;

			xPos = (int)(apmOptions->apmXPosition * width);
			yPos = (int)(apmOptions->apmYPosition * height);

			// Calculate label size
			labelRect.left  = xPos; labelRect.top = yPos; 
			labelRect.right = xPos; labelRect.bottom = yPos;
			pApmFont->DrawTextA(NULL, apmLabel, -1, &labelRect, DT_LEFT | DT_NOCLIP | DT_CALCRECT, 0xFFFFFFFF);
			boxWidth = labelRect.right - labelRect.left;
			int labelWidth = boxWidth;
			boxHeight = labelRect.bottom - labelRect.top;
			// Calculate value size
			valueRect.left = xPos + boxWidth; valueRect.top = yPos + boxHeight;
			valueRect.right = valueRect.left; valueRect.bottom = valueRect.top;
			pApmFont->DrawTextA(NULL, apmStr, -1, &valueRect, DT_LEFT | DT_NOCLIP | DT_CALCRECT, 0xFFFFFFFF);
			boxWidth += valueRect.right - valueRect.left;
			boxHeight = max(boxHeight, valueRect.bottom - valueRect.top);

			// Handle centering
			if(apmOptions->apmCenterX) xPos = (width >> 1) - (boxWidth >> 1);
			if(apmOptions->apmCenterY) yPos = (height >> 1) - (boxHeight); // because of how we fill, we need to subtract the whole boxHeight

			borderSpacing = max(min((int)(apmOptions->apmFontSize / 12), 20),2); // minimum 2px, max 20px

			labelRect.left = xPos; labelRect.right = xPos + boxWidth;
			labelRect.top = yPos; labelRect.bottom = yPos + boxHeight;
		
			valueRect.left = xPos + labelWidth; valueRect.right = xPos + boxWidth;
			valueRect.top = yPos; valueRect.bottom = yPos + boxHeight;

			lastUpdate = curTick;
		}

		// Fill background color
		FillArea(xPos-borderSpacing, yPos-borderSpacing, 
					boxWidth+(borderSpacing<<1), boxHeight+(borderSpacing<<1),
					apmOptions->apmBgColor);
		// Draw label
		pApmFont->DrawTextA(NULL, apmLabel, -1, &labelRect, DT_LEFT | DT_NOCLIP, apmOptions->apmLabelColor);
		// Draw value
		pApmFont->DrawTextA(NULL, apmStr, -1, &valueRect, DT_LEFT | DT_NOCLIP, curValueColor);
	}
}