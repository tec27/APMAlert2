#include "stdafx.h"
#include "APMAlert2.h"
#include <stdio.h>
#include <errno.h>

FILE * logFile;
HANDLE logMutex;

void initializeLogger() {
	logMutex = CreateMutex(NULL, false, "APMAlert2Logger");
	if(logMutex == NULL) {
		MessageBox(NULL, "Could not create log mutex!", "APMAlert2", MB_OK);
		return;
	}
	
	char dllPath[MAX_PATH];
	GetModuleFileName(hInstance, dllPath, MAX_PATH);
	char * pathEnd = strrchr(dllPath, '.');
	if(pathEnd == NULL) {
		strcpy_s(dllPath, MAX_PATH, "apmalert2.log");
	}
	else {
		dllPath[pathEnd-dllPath+1] = '\0';
		strcat_s(dllPath, MAX_PATH, "log");
	}
	
	errno_t ret = fopen_s(&logFile, dllPath,"a+");
	if(ret == EINVAL || logFile == NULL) {
		ret = fopen_s(&logFile, "c:\apmalert2.log", "a+");
		if(ret == EINVAL || logFile == NULL) {
			MessageBox(NULL, "Could not create log file!", "APMAlert2", MB_OK);
			CloseHandle(logMutex);
			return;
		}
	}
	fprintf(logFile, "\n");
}

void writeToLog(char * typeStr, char * str) {
	if(logFile != NULL) {
		DWORD dwWaitResult = WaitForSingleObject(logMutex, INFINITE);
		if(dwWaitResult == WAIT_OBJECT_0) {
			__try {
				SYSTEMTIME st;
				GetLocalTime(&st);
				char * lastNewline = strrchr(str, '\n');
				size_t strLen = strlen(str);
				if(str[strLen-1] == '\n') {
					str[strLen-1] = '\0';
					if(str[strLen-2] == '\r') str[strLen-2] = '\0';
				}
				fprintf(logFile, "[%02d-%02d-%04d %02d:%02d:%02d/%04d] %s - %s\n", st.wMonth, st.wDay, st.wYear, 
					st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, typeStr, str);
				fflush(logFile);
			}
			__finally {
				ReleaseMutex(logMutex);
			}
		}
	}
}

void logError(char * str) {
	writeToLog("ERROR", str);
}

void logInfo(char * str) {
	writeToLog("INFO", str);
}

void freeLogger() {
	fclose(logFile);
}