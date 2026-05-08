#pragma once

#include <map>
#include <cmath>
#include <stdint.h>
#include "helpers.h"
#include "utils.h"

struct last_command_info_t {
    // * last command number we saw for this player
    int32_t command_number;
    // * player-specific time since last update
    // todo: remove as ProcessUserCmds doesn't work with ticks
    int32_t server_tick;
};

// todo: clear on dc
static auto g_LastCommandInfo = std::map<int32_t, last_command_info_t>();

namespace Detection {
    void AddPlayer(uintptr_t player) {
        auto player_idx = CBasePlayer::GetIndex( player );

        g_LastCommandInfo[player_idx] = last_command_info_t{
            .command_number = 0,
            .server_tick = 0
        };

        print_ext_scoped( "added player %d\n", player_idx );
    }

    last_command_info_t* GetPlayerLastCommandInfo(uintptr_t player, bool* is_init) {
        auto player_idx = CBasePlayer::GetIndex( player );

        auto last_cmd_it = g_LastCommandInfo.find( player_idx );
        if ( last_cmd_it == g_LastCommandInfo.end( ) ) {
            *is_init = false;
            AddPlayer( player );
            return &g_LastCommandInfo[player_idx];
        } else {
            *is_init = true;
            return &last_cmd_it->second;
            return &last_cmd_it->second;
        }
    }

    void RemovePlayer(uintptr_t player) {
        auto player_idx = CBasePlayer::GetIndex( player );
        g_LastCommandInfo.erase( player_idx );

        print_ext_scoped( "removed player %d\n", player_idx );
    }
}