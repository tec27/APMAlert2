#include "stdafx.h"
#include "APMAlert2.h"
#include <math.h>

// New Method Idea:
// As APM readings come in, store the last time one came in
// When a new one comes in, take the (CurTick - LastTick / 60000) * APM
//  which gives you the number of actions performed in that time period
// Add this number to a total number of actions for the game
// Then do straight exponential decay on this total like APMAlert1

// Used for calculating the APM
float actions = 0.0f;
DWORD gameStartTick = 0;
DWORD lastApmUpdate = 0;
DWORD lastApm = 0;
#define APM_INTERVAL 0.30f

DWORD apm = 0;


void initializeAPMCalculator() {
	logInfo("Initializing APM calculator.");
	apm = 0;
	lastApm = 0;
	actions = 0.0f;
	gameStartTick = GetTickCount();
	lastApmUpdate = gameStartTick;
}

void calcAPM() {
	DWORD curTick = GetTickCount();

	float gameDurationFactor = 1-exp(-(int)(curTick - gameStartTick) / (APM_INTERVAL*60000));
	if(gameDurationFactor < 0.001f) gameDurationFactor = 0.001f;

	apm = (DWORD)(actions / (APM_INTERVAL*gameDurationFactor));

}

void addAPM(DWORD newAPM) {
	lastApm = newAPM;
	DWORD curTick = GetTickCount();				
	actions += ((float)(curTick - lastApmUpdate) / 60000.0f) * newAPM;
	actions *= exp(-(int)(curTick - lastApmUpdate) / (APM_INTERVAL*60000));
	lastApmUpdate = curTick;
	calcAPM();
}

void addAPM() {
	if(lastApm == 0)
		addAPM(0);
}