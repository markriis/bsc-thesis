#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"
#include "game_interfaces.h"
#include "helpers.h"

struct last_command_info_t {
    // * last command number we saw for this player
    int32_t command_number;
};

static auto g_LastCommandInfo = std::map<int32_t, last_command_info_t>();

DEFINE_VFTABLE_HOOK(
    ProcessUsercmds,
    void,
    uintptr_t thisptr, CUserCmd* cmds, int numcmds, int totalcmds, int dropped_packets, bool paused
) {
    auto player_idx = CBasePlayer::GetIndex( thisptr );

    auto last_cmd_it = g_LastCommandInfo.find( player_idx );
    auto oldest_new_cmd = &cmds[ numcmds - 1 ];

    if ( last_cmd_it == g_LastCommandInfo.end( ) ) {
        g_LastCommandInfo[player_idx] = last_command_info_t{
            .command_number = oldest_new_cmd->command_number,
        };
    }

    auto* last_cmd_data = &g_LastCommandInfo[player_idx];
    auto diff_to_oldest = oldest_new_cmd->command_number - last_cmd_data->command_number;

    // * client send less packets than we expected, add the difference to the last
    // *   command number to keep in sync with the client
    if ( diff_to_oldest > dropped_packets ) {
        // * dropped packets resulted in diff_to_oldest - 1, rather trust what data the server
        // *   has rather than blindly trusting the client to indicating us how many packets were dropped
        last_cmd_data->command_number += dropped_packets;
    } 


    // todo: if we could compare the received sequence number here
    // *       to the received command number, to validate that 
    // *       the passed command number has not been tampered with
    for ( int i = numcmds - 1; i >= 0; i-- ) {
        auto* cmd = &cmds[ i ];

        // * netchan disregards all duplicate and old packets so seqnr can
        // *   only go forward, see tf2_src\engine\net_chan.cpp
        // * command number is modifyable by client, but should also only go forward
        // *   set by a legit client as last_seqnr + choked + 1 
        auto cmd_num_delta = cmd->command_number - last_cmd_data->command_number;

        if ( cmd_num_delta != 1 ) {
            print_ext(
                "(detection) player %d cmdnum mismatch (i=%d, delta=%d) last=%d current=%d dropped=%d numcmds=%d totalcmds=%d backupcmds=%d diff_to_oldest=%d\n",
                player_idx,
                i,
                cmd_num_delta,
                last_cmd_data->command_number, cmd->command_number, dropped_packets,
                numcmds, totalcmds, totalcmds - numcmds, diff_to_oldest
            );
        }

        // * in case client chokes, we know how much difference to agree with 
        last_cmd_data->command_number = cmd->command_number;
    }

    // * let original run with (maybe) patched commands
    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_ProcessUsercmds_index );
    if ( original )
        original( thisptr, cmds, numcmds, totalcmds, dropped_packets, paused );
}