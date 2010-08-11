#include "stdafx.h"
#include "APMAlert2.h"
#include <strsafe.h>

#define hMainKey HKEY_CURRENT_USER
#define NUM_MONITORED_KEYS 3
char * monitoredKey = "Software\\Razer\\Starcraft2";
HKEY hMonitoredKey;
HANDLE hKeyEvent;

bool isIngame = false;
bool isMothershipBuilding = false;
bool isObs = true;

void HandleAPMChange() {
	DWORD temp;
	DWORD size = sizeof(temp);
	if(RegGetValue(hMonitoredKey, NULL, "APMValue", RRF_RT_REG_DWORD, NULL, &temp, &size) == ERROR_SUCCESS)
		addAPM((DWORD)(temp*1.38));
}

void HandleStartModuleChange() {
	DWORD temp;
	DWORD size = sizeof(temp);
	if(RegGetValue(hMonitoredKey, NULL, "StartModule", RRF_RT_REG_DWORD, NULL, &temp, &size) == ERROR_SUCCESS)
		isIngame = (temp == 1);
	
}

void HandleBuildingComplete() {
	if(isIngame && isObs) {
		DWORD temp;
		DWORD size = sizeof(temp);
		if(RegGetValue(hMonitoredKey, NULL, "BuildingComplete", RRF_RT_REG_DWORD, NULL, &temp, &size) == ERROR_SUCCESS)
			isObs = !(temp == 1);
	}
}

void (*keyChangeCallbacks[NUM_MONITORED_KEYS])(void) = {
	HandleAPMChange,
	HandleStartModuleChange,
	HandleBuildingComplete
};

RazerMonitorThread::RazerMonitorThread() {}

void RazerMonitorThread::Setup() {
	LONG lError;
	lError = RegOpenKeyEx(hMainKey, monitoredKey, 0, KEY_READ | KEY_NOTIFY, &hMonitoredKey);
	if(lError != ERROR_SUCCESS) {
		ErrorMsg("RegOpenKey");
		setTerminated(true);
		return;
	}
	
	hKeyEvent = CreateEvent(NULL, false, false, "APMAlert2RegMonitor");
	if(hKeyEvent == NULL) {
		ErrorMsg("CreateEvent");
		setTerminated(true);
	}
	logInfo("Registry monitor thread setup completed.");
}

void RazerMonitorThread::Execute(void * data) {
	bool rereg = true;
	while(!isTerminated()) {
		if(rereg) {
			LONG lError = RegNotifyChangeKeyValue(hMonitoredKey, true, REG_NOTIFY_CHANGE_LAST_SET, hKeyEvent, true);
			if(lError != ERROR_SUCCESS) {
				ErrorMsg("RegNotifyChangeKeyValue");
				setTerminated(true);
			}
			rereg = false;
		}

		DWORD ret = WaitForSingleObject(hKeyEvent, 1000);
		if(ret != WAIT_FAILED && ret != WAIT_TIMEOUT) {
			for(int i = 0; i < NUM_MONITORED_KEYS; i++)
				keyChangeCallbacks[i]();
			rereg = true;
		}
		else if(ret == WAIT_TIMEOUT) {
			addAPM();
		}
	}

	if(hKeyEvent != NULL)
		CloseHandle(hKeyEvent);
	if(hMonitoredKey != NULL)
		RegCloseKey(hMonitoredKey);
}