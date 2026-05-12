#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"
#include "game_interfaces.h"
#include "helpers.h"
#include "detection.h"

#include <map>
#include <cmath>



// * tf2_src\engine\sv_client.cpp#L166
// $ int totalcmds =msg->m_nBackupCommands + msg->m_nNewCommands;
// * numcdmds passed as
// $ msg->m_nNewCommands
DEFINE_VFTABLE_HOOK(
    ProcessUsercmds,
    void,
    uintptr_t thisptr, CUserCmd* cmds, int numcmds, int totalcmds, int dropped_packets, bool paused
) {
    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_ProcessUsercmds_index );
    
    // * commands are added to context via iterating (totalcmds-1)..0
    // * - going from oldest backup to newest (last totalcmds - numcmds entries)
    // * - going from oldest new command to newest new command (0..numcmds-1 entries)
    // processed in tf2_src\game\server\player.cpp

    // * fakelag is based on buffering commands, and engine design choices
    // *  kind of get in the way of patching this cheat overall
    // * as the player is simulated on the server by processing all of the
    // *  received commands in one go, it's inherently impossible to patch
    // *  fakelag, without limiting the maximum allowed choke amount
    if ( numcmds > 5 ){
        auto backup_cmds = totalcmds - numcmds;
        numcmds = 5;
        totalcmds = backup_cmds + numcmds;
    }

    if ( original )
        original( thisptr, cmds, numcmds, totalcmds, dropped_packets, paused );
}