#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"
#include "game_interfaces.h"
#include  "helpers.h"

#include <map>
#include <cmath>

struct last_command_info_t {
    // * last command number we saw for this player
    int32_t command_number;
    // * player-specific time since last update
    int32_t server_tick;
};

static auto g_LastCommandInfo = std::map<int32_t, last_command_info_t>();

DEFINE_VFTABLE_HOOK(
    PlayerRunCommand,
    void,
    uintptr_t thisptr, CUserCmd* cmd, void* movehelper
) { 
    //todo: isalive check
    auto player_idx = CBasePlayer::GetIndex( thisptr );

    auto last_cmd_it = g_LastCommandInfo.find( player_idx );
    if ( last_cmd_it == g_LastCommandInfo.end( ) ) {
        g_LastCommandInfo[player_idx] = last_command_info_t{
            .command_number = cmd->command_number,
            .server_tick = GameInterfaces::g_pGlobals->tickcount - 1
        };
    }
    auto* last_cmd_data = &g_LastCommandInfo[player_idx];

    // * did client choke inbetween?
    const auto tick_delta = GameInterfaces::g_pGlobals->tickcount - last_cmd_data->server_tick;
    // * sequence number change
    const auto cmd_num_delta = cmd->command_number - last_cmd_data->command_number;

    // * cmd_num_delta can be 0 btw, think it was caused by batching commands
    if ( cmd_num_delta > tick_delta + 1 || cmd_num_delta < 0 ) {
        print_ext( "(detection) player %d cmdnum went backwards last=%d current=%d (last command %d tick%s ago)\n", player_idx, last_cmd_data->command_number, cmd->command_number, tick_delta, tick_delta > 1 ? "s" : "" );
    }

    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_PlayerRunCommand_index );
    if ( original )
        original( thisptr, cmd, movehelper );

    // * in case client chokes, we know how much difference to agree with 
    last_cmd_data->server_tick = GameInterfaces::g_pGlobals->tickcount;
    last_cmd_data->command_number = cmd->command_number;
}