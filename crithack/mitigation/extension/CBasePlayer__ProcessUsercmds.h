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


// numcmds = new cmds in msg packet (1..totalcmds, newes..oldest)
// totalcmds = backup + cmd count
DEFINE_VFTABLE_HOOK(
    ProcessUsercmds,
    void,
    uintptr_t thisptr, CUserCmd* cmds, int numcmds, int totalcmds, int dropped_packets, bool paused
) {
    // baseline is currently being taken from cache
    // fix: skip cached ones, use new commands to set a base
    bool player_init = false;
    auto last_cmd_data = Detection::GetPlayerLastCommandInfo( thisptr, &player_init );

    CUserCmd* oldest_new = &cmds[numcmds - 1];

    if ( !player_init ) {
        last_cmd_data->command_number = oldest_new->command_number - 1;
        print_ext_scoped( "initializing last command data for player %d, command_number=%d\n", CBasePlayer::GetIndex( thisptr ), last_cmd_data->command_number );
    }

    //todo: maybe want to use dropped_packets
    // go through all new packets
    for ( int i = numcmds - 1; i >= 0; i-- ) {
        CUserCmd* cmd = &cmds[ i ];
        
        // * sequence number change
        const auto cmd_num_delta = cmd->command_number - last_cmd_data->command_number;

        // * cmd_num_delta can be 0 btw, think it was caused by batching commands
        if ( cmd_num_delta > dropped_packets + 1 || cmd_num_delta < 0 ) {
            auto player_idx = CBasePlayer::GetIndex( thisptr );            
            
            // * nuh uh cheater
            const auto old_cmdnum = cmd->command_number;
            cmd->command_number = last_cmd_data->command_number + std::max(1, dropped_packets);
            // * need to set random seed aswell, otherwise fully trusting client
            cmd->random_seed__0x38 = GameFunctions::MD5_PseudoRandom( cmd->command_number ) & 0x7FFFFFFF;
            cmd->server_random_seed__0x3C = cmd->random_seed__0x38;
            // print_ext( "patched cmdnum on player %d, last=%d new=%d (was %d) dropped_packets=%d\n", player_idx, last_cmd_data->command_number, cmd->command_number, old_cmdnum, dropped_packets );
        }

        // * in case client chokes, we know how much difference to agree with 
        last_cmd_data->command_number = cmd->command_number;
    }
    
    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_ProcessUsercmds_index );
    if ( original )
        original( thisptr, cmds, numcmds, totalcmds, dropped_packets, paused );
}