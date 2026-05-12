#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"

/*
ida pseudocode typedef said:
void __fastcall CBasePlayer::ProcessUsercmds(
        CBasePlayer *this,
        CUserCmd *a2,
        signed int a3,
        signed int a4,
        unsigned int a5,
        char a6,
        __m128i a7)
{

but in CServerGameClients::ProcessUsercmds decomp it gets called as:
(*(void (__fastcall **)(__int64, __int64 *, _QWORD, _QWORD, _QWORD, bool))(*(_QWORD *)plr_baseentity + 3448LL))(
      plr_baseentity,
      cmds,
      (unsigned int)numcmds,
      (unsigned int)totalcmds,
      (unsigned int)dropped_packets,
      paused);
*/

DEFINE_VFTABLE_HOOK(
    ProcessUsercmds,
    void,
    void* thisptr, CUserCmd* cmds, int numcmds, int totalcmds, int dropped_packets, bool paused
) {
    // last solution didn't assign original as hook structure was different
    // originals for players are now stored under PlayerHookManager, so get original from there and call it if exists
    // could later name it to a general hook manager and store via string keys
    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_ProcessUsercmds_index );

    if ( original )
        original( thisptr, cmds, numcmds, totalcmds, dropped_packets, paused );

    // print_ext_scoped(
    //     "OnProcessUsercmds_Post | numcmds=%d, totalcmds=%d, dropped_packets=%d, paused=%d\n",
    //     numcmds, totalcmds, dropped_packets, paused
    // );
};