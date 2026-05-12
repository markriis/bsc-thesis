#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"
#include "game_interfaces.h"

#include <map>
#include <cmath>

struct run_command_player_info {
    int last_update_time;
    std::vector< CUserCmd > cmds_simulated;

    Vector start_pos;
};

// plr ptr to last updated time
static auto g_LastUpdateTimes = std::map< uintptr_t, run_command_player_info >( );

DEFINE_VFTABLE_HOOK(
    PlayerRunCommand,
    void,
    uintptr_t thisptr, CUserCmd* cmd, void* movehelper
) {    
    auto tickcount = GameInterfaces::g_pGlobals->tickcount;

    // * inlined vector, 0x3C4, 0x3C8, 0x3CC for x,y,z respectively
    auto pos_addr = ((char *)thisptr + CBasePlayer_mVecAbsOrigin_offset);
    auto pos_x = *reinterpret_cast< float* >( pos_addr );
    auto pos_y = *reinterpret_cast< float* >( pos_addr + 4 );
    auto pos_z = *reinterpret_cast< float* >( pos_addr + 8 );
    

    if ( g_LastUpdateTimes.find( thisptr ) == g_LastUpdateTimes.end( ) ) {
        g_LastUpdateTimes[ thisptr ] = run_command_player_info{
            tickcount, { },
            Vector{ pos_x, pos_y, pos_z }
        };
    } else {
        auto& info = g_LastUpdateTimes[ thisptr ];

        // * new simulation
        // * kind of a hacky way to log simulation cmds processed per batch as I am not
        // *   skilled enough to implement trampoline hooks to use in CBasePlayer::PhysicsSimulate,
        // *   but since server simulates players on an interval tickcount should be fine?
        // *   otherwise could also use simtime or some other global var (frametime/curtime)
        if ( info.last_update_time != tickcount ) {
            auto time_delta = tickcount - info.last_update_time;

            auto pos_diff = Vector{
                pos_x - info.start_pos.x,
                pos_y - info.start_pos.y,
                pos_z - info.start_pos.z
            };

            auto diff_dist = sqrtf( pos_diff.x * pos_diff.x + pos_diff.y * pos_diff.y + pos_diff.z * pos_diff.z );

            std::string cmd_fmt;
            // "simulated cmds:\0" + 12 chars per cmd number
            cmd_fmt.reserve( 16 + info.cmds_simulated.size( ) * 12 );

            cmd_fmt = "simulated cmds:";
            for ( const auto& simulated_cmd : info.cmds_simulated ) {
                cmd_fmt += " ";
                cmd_fmt += std::to_string( simulated_cmd.command_number );
            }

            print_ext_scoped( "OnPlayerRunCommand | (new tick) simulated %d cmds in %d ticks, player moved %.2f hu\n\t%s", info.cmds_simulated.size( ), time_delta, diff_dist, cmd_fmt.c_str() );
            

            // * and clear cached cmds
            info.cmds_simulated.clear( );

            // * update pos
            info.start_pos.x = pos_x;
            info.start_pos.y = pos_y;
            info.start_pos.z = pos_z;

            // * update last update time
            info.last_update_time = tickcount;
        }
        
        // * push current cmd
        info.cmds_simulated.push_back( *cmd );
    }

    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_PlayerRunCommand_index );

    if ( original )
        original( thisptr, cmd, movehelper );
}