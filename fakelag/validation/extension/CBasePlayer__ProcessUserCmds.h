#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "ida_defines.h"
#include "extension.h"
#include "utils.h"

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
MAKE_VFTABLE_HOOK(
    ProcessUsercmds,
    void,
    void* thisptr, CUserCmd* cmd, int numcmds, int totalcmds, int dropped_packets, bool paused
) {
    // codex start
    print_ext("HIT ProcessUsercmds this=%p cmds=%p numcmds=%d totalcmds=%d dropped=%d paused=%d original=%p\n",
        thisptr, cmd, numcmds, totalcmds, dropped_packets, paused, (void*)original);

    // original is asked from the namespace, our original is stored in player vft
    if (original)
        original(thisptr, cmd, numcmds, totalcmds, dropped_packets, paused);
    // codex end

    // original( thisptr, cmd, numcmds, totalcmds, dropped_packets, paused );

    g_HookHelper.OnProcessUsercmds_Post( thisptr, cmd, numcmds, totalcmds, dropped_packets, paused );
};