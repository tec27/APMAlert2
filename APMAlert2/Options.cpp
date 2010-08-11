#include "stdafx.h"
#include "APMAlert2.h"
#include "stdio.h"

D3DCOLOR strAlphaToColor(char * str, float alpha) {
	unsigned int color; 
	if(str[0] == '#') str++;
	sscanf_s(str, "%x", &color);

	float r = ((color >> 16) & 0xFF) / 255.0f;
	float g = ((color >> 8) & 0xFF) / 255.0f;
	float b = ((color)      & 0xFF) / 255.0f;

	return D3DCOLOR_COLORVALUE(r, g, b, alpha);
}

APMAlertOptions * LoadAPMAlertOptions() {
	logInfo("Loading APMAlert2 Options...");
	APMAlertOptions * opts = (APMAlertOptions*)malloc(sizeof(APMAlertOptions));

	// Load the default settings
	opts->minAPM = 120;
	opts->alertKey = 0x700000dc; // CTRL+ALT+SHIFT+BackSlash
	opts->alertSound = NULL;
	opts->alertEnabled = true;

	opts->clockColor = 0xFFFFFFFF;
	opts->clockBgColor = 0x99000000;
	float clockBgAlpha = 0.6f;
	opts->clockFontName = _strdup("Arial");
	opts->clockFontSize = 24.0f;
	opts->clockFontBold = true;
	opts->clockFontItalic = false;
	opts->clockXPosition = 0.0f;
	opts->clockYPosition = 0.026f;
	opts->clockCenterX = true;
	opts->clockCenterY = false;
	opts->clockEnabled = true;

	opts->apmLabelColor = 0xFFFFFFFF;
	opts->apmBgColor = 0x99000000;
	float apmBgAlpha = 0.6f;
	opts->apmFontName = _strdup("Arial");
	opts->apmFontSize = 15.75f;
	opts->apmFontBold = true;
	opts->apmFontItalic = false;
	opts->apmXPosition = 0.0035f;
	opts->apmYPosition = 0.026f;
	opts->apmCenterX = false;
	opts->apmCenterY = false;
	opts->apmRangeBottom0 = 0;
	opts->apmRangeBottom1 = 0;
	opts->apmRangeBottom2 = 0;
	opts->apmRangeBottom3 = 0;
	opts->apmRangeBottom4 = 0;
	opts->apmRangeColor0 = 0xFFFFFFFF;
	opts->apmRangeColor1 = 0xFFFFFFFF;
	opts->apmRangeColor2 = 0xFFFFFFFF;
	opts->apmRangeColor3 = 0xFFFFFFFF;
	opts->apmRangeColor4 = 0xFFFFFFFF;
	opts->apmEnabled = true;

	logInfo("Default options set, loading settings from registry.");

	// Load new settings out of the registry
	HKEY hKey;
	LONG lError;
	lError = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\tec27\\APMAlert2", 0, KEY_READ | KEY_NOTIFY, &hKey);
	if(lError != ERROR_SUCCESS) {
		ErrorMsg("LoadSettings/RegOpenKey");
		return opts;
	}
	
	DWORD tempDword;
	char tempStr[MAX_PATH];
	DWORD size;

	// Please forgive me for this horribly ugly code. Actually, just forget this file exists.
	// DWORDS:
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "MinAPM", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->minAPM = tempDword;
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "AlertEnabled", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->alertEnabled = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "AlertKey", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->alertKey = tempDword;
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "ClockFontBold", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->clockFontBold = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "ClockFontItalic", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->clockFontItalic = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "ClockCenterX", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->clockCenterX = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "ClockCenterY", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->clockCenterY = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "ClockEnabled", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->clockEnabled = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMFontBold", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmFontBold = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMFontItalic", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmFontItalic = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMCenterX", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmCenterX = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMCenterY", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmCenterY = (tempDword == 1);
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMGr1", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmRangeBottom1 = tempDword;
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMGr2", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmRangeBottom2 = tempDword;
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMGr3", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmRangeBottom3 = tempDword;
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMGr4", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmRangeBottom4 = tempDword;
	size = sizeof(tempDword);
	if(RegGetValue(hKey, NULL, "APMEnabled", RRF_RT_REG_DWORD, NULL, &tempDword, &size) == ERROR_SUCCESS)
		opts->apmEnabled = (tempDword == 1);

	// Floats (stored in strings)
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMBgAlpha", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		apmBgAlpha = (float)atof(tempStr);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "ClockBgAlpha", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		clockBgAlpha = (float)atof(tempStr);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMFontSize", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmFontSize = (float)atof(tempStr);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "ClockFontSize", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->clockFontSize = (float)atof(tempStr);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMXPosition", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmXPosition = (float)atof(tempStr);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMYPosition", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmYPosition = (float)atof(tempStr);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "ClockXPosition", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->clockXPosition = (float)atof(tempStr);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "ClockYPosition", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->clockYPosition = (float)atof(tempStr);

	// Colors (stored in strings)
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMBgColor", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmBgColor = strAlphaToColor(tempStr, apmBgAlpha);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMLabelColor", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmLabelColor = strAlphaToColor(tempStr, 1.0f);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMColorGr0", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmRangeColor0 = strAlphaToColor(tempStr, 1.0f);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMColorGr1", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmRangeColor1 = strAlphaToColor(tempStr, 1.0f);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMColorGr2", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmRangeColor2 = strAlphaToColor(tempStr, 1.0f);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMColorGr3", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmRangeColor3 = strAlphaToColor(tempStr, 1.0f);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMColorGr4", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->apmRangeColor4 = strAlphaToColor(tempStr, 1.0f);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "ClockBgColor", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->clockBgColor = strAlphaToColor(tempStr, clockBgAlpha);
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "ClockColor", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS)
		opts->clockColor = strAlphaToColor(tempStr, 1.0f);

	// Strings (stored in DWORDS) (Just kidding, they're stored in strings, silly)
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "AlertSound", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS) {
		free(opts->alertSound);
		opts->alertSound = _strdup(tempStr);
	}
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "APMFontName", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS) {
		free(opts->apmFontName);
		opts->apmFontName = _strdup(tempStr);
	}
	size = MAX_PATH;
	if(RegGetValue(hKey, NULL, "ClockFontName", RRF_RT_REG_SZ, NULL, tempStr, &size) == ERROR_SUCCESS) {
		free(opts->clockFontName);
		opts->clockFontName = _strdup(tempStr);
	}

	logInfo("Options loaded correctly.");

	return opts;
}

void FreeAPMAlertOptions(APMAlertOptions * opts) {
	logInfo("Freeing options structure.");

	free(opts->alertSound);
	opts->alertSound = NULL;
	free(opts->clockFontName);
	opts->clockFontName = NULL;
	free(opts->apmFontName);
	opts->apmFontName = NULL;
	free(opts);
}
