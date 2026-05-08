#pragma once

/*
* reversing of FX_FireBullets shows that StartLagCompensation is thiscalled
~ TE_FireBullets(v19, (const Vector *)a3, a4, iWeaponID, a6, a9, a7, a10);
~ CTFPlayer::NoteWeaponFired((CTFPlayer *)pPlayer);
~ lagcompensation->vft->StartLagCompensation(
~    lagcompensation,
~    (CBasePlayer *)pPlayer,
~    *(CUserCmd **)(pPlayer + 4368));
~ if ( g_pPasstimeLogic && CTFPasstimeLogic::GetBall((CTFPasstimeLogic *)g_pPasstimeLogic) )
* weirdly enough, StartLagCompensation is the first func in the vft even though vft for
*   CLagCompensationmanager has StartLagCompensation at index 19 & 23 (virtual thunk)
* buuuuut global is defined in source as ILagCompensationManager and same variable is later set
*   as CLagCompensationManager object 
*/

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "offsets.h"
#include "game_interfaces.h"
#include "extension.h"
#include "helpers.h"
#include "detection.h"

#include <map>
#include <cmath>

using CBasePlayer__IsBot_t = bool(*)(uintptr_t);

DEFINE_VFTABLE_HOOK(
    StartLagCompensation,
    void,
    uintptr_t thisptr, uintptr_t plr, CUserCmd* cmd
) {
    // from void __fastcall CPlayerInfo::SetAbsAngles(CPlayerInfo *this, QAngle *ang)
    // ~ v4 = *(__int64 (__fastcall **)(CBasePlayer *__hidden))(*(_QWORD *)m_pParent + 3648LL);
    // ~ v4 -> CBasePlayer::IsBot
    CBasePlayer__IsBot_t CBasePlayer__IsBot = (CBasePlayer__IsBot_t)(*(uintptr_t*)(*(uintptr_t*)plr + 3648));

    // * is the player a human
    if( !CBasePlayer__IsBot( plr ) ) {
        int32_t plr_index = CBasePlayer::GetIndex( plr );

        // * did cmd tick count go backwards?
        const auto last_cmd_it = g_LastSimulatedCmd.find( plr_index );
        if ( last_cmd_it == g_LastSimulatedCmd.end( ) )
            goto original;

        const auto& last_cmd = last_cmd_it->second;
            
        // * tickcount increased, ignore
        if ( last_cmd.tick_count < cmd->tick_count ) 
            goto original;

        auto nci = (CNetChannel*)GameInterfaces::g_pEngineServer->vft->GetPlayerNetInfo(GameInterfaces::g_pEngineServer, plr_index + 1);

        // ? netchannel info should always be valid here
        if ( !nci )
            goto original;
    
        // * estimate correct tickcount for command 
        float plr_latency = nci->vft->GetLatency((INetChannelInfo *)nci, FLOW_OUTGOING);

        int32_t plr_latency_ticks = TIME_TO_TICKS( plr_latency );
        int32_t estimated_tickcount = GameInterfaces::g_pGlobals->tickcount - plr_latency_ticks;

        int latency_ms = static_cast< int >( plr_latency * 1000 );
        const auto old_tc = cmd->tick_count;
        // * don't allow going backwards with estimation
        if ( estimated_tickcount < last_cmd.tick_count ) {
            cmd->tick_count = last_cmd.tick_count + 1;
            print_ext_scoped( "(mitigation) client %d (%dms), method=1, old_tc=%d new_tc=%d last_tc=%d estimated_tc=%d\n", plr_index, latency_ms, old_tc, cmd->tick_count, last_cmd.tick_count, estimated_tickcount );
        } else {
            cmd->tick_count = estimated_tickcount;
            print_ext_scoped( "(mitigation) client %d (%dms), method=2, old_tc=%d new_tc=%d last_tc=%d estimated_tc=%d\n", plr_index, latency_ms, old_tc, cmd->tick_count, last_cmd.tick_count, estimated_tickcount );
        }
    }

original:
    if ( original )
        original( thisptr, plr, cmd );
}