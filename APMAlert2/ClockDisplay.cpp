#include "stdafx.h"
#include "APMAlert2.h"
#include <strsafe.h>

const char * clockFormat = "%02d:%02d %s";
#define UPDATE_DELTA 1000 // How many milliseconds between updating the time info
DWORD lastTick = 0;
SYSTEMTIME st;
char clockStr[12];

int clockXPos = 0;
int clockYPos = 0;
int clockBorderSpacing = 0;
int clockBoxWidth = 0, clockBoxHeight = 0;
RECT clockRect;

void drawClock(IDirect3DDevice9 * ppD3DDevice, IDirect3DSwapChain9 * pSwapChain, UINT width, UINT height) {
	if(pClockFont != NULL && pLine != NULL && apmOptions->clockEnabled) {
		DWORD curTick = GetTickCount();
		if(curTick - lastTick > UPDATE_DELTA) {
			GetLocalTime(&st);
			int hour = st.wHour%12;
			if(hour == 0) hour = 12;
			sprintf_s(clockStr, 12, clockFormat, hour, st.wMinute, (st.wHour >= 12 ? "PM" : "AM"));

			clockXPos = (int)(apmOptions->clockXPosition * width);
			clockYPos = (int)(apmOptions->clockYPosition * height);

			// Calculate label size
			clockRect.left  = clockXPos; clockRect.top = clockYPos; 
			clockRect.right = clockXPos; clockRect.bottom = clockYPos;
			pClockFont->DrawTextA(NULL, clockStr, -1, &clockRect, DT_LEFT | DT_NOCLIP | DT_CALCRECT, 0xFFFFFFFF);
			clockBoxWidth = clockRect.right - clockRect.left;
			clockBoxHeight = clockRect.bottom - clockRect.top;

			// Handle centering
			if(apmOptions->clockCenterX) clockXPos = (width >> 1) - (clockBoxWidth >> 1);
			if(apmOptions->clockCenterY) clockYPos = (height >> 1) - (clockBoxHeight); // because of how we fill, we need to subtract the whole boxHeight

			clockBorderSpacing = max(min((int)(apmOptions->clockFontSize / 12), 20),2); // minimum 2px, max 20px

			clockRect.left = clockXPos; clockRect.right = clockXPos + clockBoxWidth;
			clockRect.top = clockYPos; clockRect.bottom = clockYPos + clockBoxHeight;

			lastTick = curTick;
		}
		// Fill background color
		FillArea(clockXPos-clockBorderSpacing, clockYPos-clockBorderSpacing, 
					clockBoxWidth+(clockBorderSpacing<<1), clockBoxHeight+(clockBorderSpacing<<1),
					apmOptions->clockBgColor);
		// Draw label
		pClockFont->DrawTextA(NULL, clockStr, -1, &clockRect, DT_LEFT | DT_NOCLIP, apmOptions->clockColor);
	}
}