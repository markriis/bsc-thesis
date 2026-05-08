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

DEFINE_VFTABLE_HOOK(
    PlayerRunCommand,
    void,
    uintptr_t thisptr, CUserCmd* cmd, void* movehelper
) {
    uint32_t plr_index = CBasePlayer::GetIndex( thisptr );

    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_PlayerRunCommand_index );
    
    if ( original )
        original( thisptr, cmd, movehelper );

    // * no stored last cmd?
    if (g_LastSimulatedCmd.find( plr_index ) == g_LastSimulatedCmd.end( ) ) {
        g_LastSimulatedCmd[ plr_index ] = { };
    }

    // * copy command over
    CopyUserCmd( &g_LastSimulatedCmd[ plr_index ], cmd );
}