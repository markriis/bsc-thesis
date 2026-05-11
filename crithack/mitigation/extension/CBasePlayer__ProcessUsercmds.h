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
// netchan keeps track of dropped packets 
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

    auto diff_to_oldest = oldest_new->command_number - last_cmd_data->command_number;

    if ( diff_to_oldest > dropped_packets ) {
        // * dropped packets resulted in diff_to_oldest - 1, rather trust what data the server
        // *   has rather than blindly trusting the client to indicating us how many packets were dropped
        last_cmd_data->command_number += dropped_packets;
    } 

    for ( int i = numcmds - 1; i >= 0; i-- ) {
        CUserCmd* cmd = &cmds[ i ];
        
        // * sequence number change
        const auto cmd_num_delta = cmd->command_number - last_cmd_data->command_number;

        // * cmd_num_delta can be 0 btw, think it was caused by batching commands
        if ( cmd_num_delta != 1 ) {
            auto player_idx = CBasePlayer::GetIndex( thisptr );            
            
            // * nuh uh cheater
            const auto old_cmdnum = cmd->command_number;
            cmd->command_number = last_cmd_data->command_number + 1;

            // * need to set random seed aswell, otherwise fully trusting client
            cmd->random_seed__0x38 = GameFunctions::MD5_PseudoRandom( cmd->command_number ) & 0x7FFFFFFF;
            // * actually, don't want to touch this as this is only used for calculating random spread
            // *   let the server handle setting it, otherwise we would introduce a new exploit
            // *   to enable nospread
            // cmd->server_random_seed__0x3C = cmd->random_seed__0x38;
            print_ext(
                "(mitigation) patched cmdnum on player %d, last=%d new=%d (was %d) dropped_packets=%d numcmds=%d totalcmds=%d backupcmds=%d diff_to_oldest=%d\n",
                player_idx,
                last_cmd_data->command_number, cmd->command_number,
                old_cmdnum,
                dropped_packets,
                numcmds,
                totalcmds,
                totalcmds - numcmds,
                diff_to_oldest
            );
        }

        // * update the last command number we saw for this player
        last_cmd_data->command_number = cmd->command_number;
    }
    
    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_ProcessUsercmds_index );
    if ( original )
        original( thisptr, cmds, numcmds, totalcmds, dropped_packets, paused );
}