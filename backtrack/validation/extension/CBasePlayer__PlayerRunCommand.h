#pragma once

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"
#include "game_interfaces.h"

#include <map>
#include <cmath>

#define MAX_TICKCOUNT_DELTA 4

DEFINE_VFTABLE_HOOK(
    PlayerRunCommand,
    void,
    uintptr_t thisptr, CUserCmd* cmd, void* movehelper
) {    
    // * idea:
    // * when cmd is attacking, check if cmd tick count is close to server's
    // *   est_tick = tick count - plr latency
    // * 
    // * as when a player shoots, we want to be close to the player's client tick_count
    // *   -> we estinamte it, if its not in delta, we can modify the cmd tick count to
    // *   be the estimated tick count

    /*
    from
    void __fastcall CLagCompensationManager::StartLagCompensation(
        CLagCompensationManager *this,
        CBasePlayer *plr,
        CUserCmd *cmd)

            plr_index = 0LL;
            v10 = *((_QWORD *)plr + 8);
            if ( v10 )
              plr_index = (unsigned int)*(__int16 *)(v10 + 6);
            nci = (CNetChannel *)engine->vft->GetPlayerNetInfo(engine, plr_index);
            if ( nci )
              correct = nci->vft->GetLatency((INetChannelInfo *)nci, FLOW_OUTGOING);
            else
              correct = 0.0;
            interval_per_tick__0x1C = gpGlobals->interval_per_tick__0x1C;
            tickcount = gpGlobals->tickcount;
            lerpticks = (int)(float)((float)(*((float *)plr + 900) / interval_per_tick__0x1C) + 0.5);
    */


    /*
    // * is the player trying to attack?
    bool attacking = ( cmd->buttons & IN_ATTACK ) != 0;
    // todo: needs to check if can fire , maybe move to CLagCompensationManager::BacktrackPlayer
    if ( attacking ) {
        // * copied straight from ida
        auto plr_index = 0LL;
        auto v10 = *reinterpret_cast< uintptr_t* >( thisptr + 8 );
        if ( v10 )
            plr_index = *reinterpret_cast< int16_t* >( v10 + 6 );
        
        // todo: fails 247 :p
        auto nci = (CNetChannel*)GameInterfaces::g_pEngineServer->vft->GetPlayerNetInfo(GameInterfaces::g_pEngineServer, plr_index);
        
        if (nci) {
            float plr_latency = nci ? nci->vft->GetLatency((INetChannelInfo *)nci, FLOW_OUTGOING) : 0.0f;

            int32_t plr_latency_ticks = TIME_TO_TICKS( plr_latency );
            int32_t estimated_tickcount = cmd->tick_count - plr_latency_ticks;

            // * std::abs was yelling at me
            int32_t diff = plr_latency_ticks > cmd->tick_count
                    ? plr_latency_ticks - cmd->tick_count
                    : cmd->tick_count - plr_latency_ticks;

            if ( diff > MAX_TICKCOUNT_DELTA ) {
                int latency_ms = static_cast<int>(plr_latency * 1000);
                print_ext_scoped( "client %d (%dms) cmd tickcount %d is too far from estimated tickcount %d, correcting\n", plr_index, latency_ms, cmd->tick_count, estimated_tickcount );
                cmd->tick_count = estimated_tickcount;
            }
        } else {
            print_ext_scoped( "client %d has no net channel info, cannot correct tickcount\n", plr_index );
        }
        
        
    }
        */
    

    auto original = g_PlayerHookManager.GetOriginal< def >( CBasePlayer_PlayerRunCommand_index );

    if ( original )
        original( thisptr, cmd, movehelper );
}