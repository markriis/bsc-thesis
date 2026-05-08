#pragma once

#include "sdk_defines.h"

namespace GameInterfaces {
    CGlobalVars* g_pGlobals = nullptr;
    CEngineServer* g_pEngineServer = nullptr;
    CLagCompensationManager* g_pLagCompensationManager = nullptr;
}

// ~           lerpticks = (int)(float)((float)(*((float *)plr + 900) / interval_per_tick__0x1C) + 0.5);
#define TIME_TO_TICKS( time ) ( (int)( ( time ) / GameInterfaces::g_pGlobals->interval_per_tick__0x1C + 0.5f ) )