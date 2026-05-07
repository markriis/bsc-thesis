#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"
#include "game_interfaces.h"
#include "helpers.h"

#include <map>
#include <cmath>


DEFINE_VFTABLE_HOOK(
    PlayerRunCommand,
    void,
    uintptr_t thisptr, CUserCmd* cmd, void* movehelper
) {
    uint32_t plr_index = CBasePlayer::GetIndex( thisptr );

    g_CurrentTickCmd[ plr_index ] = cmd->tick_count;

    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_PlayerRunCommand_index );

    if ( cmd->tick_count < g_PreviousTickCmd[ plr_index ] )
        print_ext(
            "(continuity error) player %d with tick count %d (prev=%d delta=%dt)\n",
            plr_index, cmd->tick_count, g_PreviousTickCmd[ plr_index ], cmd->tick_count - g_PreviousTickCmd[ plr_index ]
        );

    // * let simulation happen before we cache
    if ( original )
        original( thisptr, cmd, movehelper );
    
    g_PreviousTickCmd[ plr_index ] = cmd->tick_count;
}