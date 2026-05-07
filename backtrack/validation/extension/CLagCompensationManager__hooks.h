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

#include <map>
#include <cmath>

#define MAX_TICKCOUNT_DELTA 4

using CBasePlayer__IsBot_t = bool(*)(uintptr_t);

DEFINE_VFTABLE_HOOK(
    CLagCompensationManager__StartLagCompensation,
    void,
    uintptr_t thisptr, uintptr_t plr, CUserCmd* cmd
) {
    // from void __fastcall CPlayerInfo::SetAbsAngles(CPlayerInfo *this, QAngle *ang)
    // ~ v4 = *(__int64 (__fastcall **)(CBasePlayer *__hidden))(*(_QWORD *)m_pParent + 3648LL);
    // ~ v4 -> CBasePlayer::IsBot
    CBasePlayer__IsBot_t CBasePlayer__IsBot = (CBasePlayer__IsBot_t)(*(uintptr_t*)(*(uintptr_t*)plr + 3648));

    // * is the player a human
    if( !CBasePlayer__IsBot( plr ) ) {
        // * copied straight from ida
        /*
        ~ *((_BYTE *)this + 88948) = 1;
        ~ plr_index = 0LL;
        ~ v10 = *((_QWORD *)plr + 8);
        ~ if ( v10 )
        ~     plr_index = (unsigned int)*(__int16 *)(v10 + 6);
        ~ nci = (CNetChannel *)engine->vft->GetPlayerNetInfo(engine, plr_index);
        ~ if ( nci )
        ~     correct = nci->vft->GetLatency((INetChannelInfo *)nci, FLOW_OUTGOING);
        ~ else
        ~     correct = 0.0;
        ~ interval_per_tick = gpGlobals->interval_per_tick__0x1C;
        */
       
        int32_t plr_index = 0;
        auto v10 = *reinterpret_cast< uintptr_t* >( thisptr + 8 );
        if ( v10 )
            // movsx   esi, word ptr [rax+6]
            plr_index = *reinterpret_cast< int16_t* >( v10 + 6 ) + 1;
        
        
        /* LOL this is why its + 1
        $ virtual INetChannelInfo* GetPlayerNetInfo( int playerIndex )
        $ {
        $    if ( playerIndex < 1 || playerIndex > sv.GetClientCount() )
        $        return NULL;
        $
        $    CGameClient *client = sv.Client( playerIndex - 1 );
        $    return client->m_NetChannel;   
        $ }    
        */
        auto nci = (CNetChannel*)GameInterfaces::g_pEngineServer->vft->GetPlayerNetInfo(GameInterfaces::g_pEngineServer, plr_index);

        if ( nci ) {
            float plr_latency = nci->vft->GetLatency((INetChannelInfo *)nci, FLOW_OUTGOING);

            int32_t plr_latency_ticks = TIME_TO_TICKS( plr_latency );
            int32_t estimated_tickcount = GameInterfaces::g_pGlobals->tickcount - plr_latency_ticks;

            // * std::abs was yelling at me
            int32_t diff = estimated_tickcount > cmd->tick_count
                    ? estimated_tickcount - cmd->tick_count
                    : cmd->tick_count - estimated_tickcount;

            // on 200ms ping, off by ~6 ticks on first record
            if ( diff > MAX_TICKCOUNT_DELTA ) {
                int latency_ms = static_cast< int >( plr_latency * 1000 );
                print_ext_scoped( "client %d (%dms) cmd tickcount %d is too far from estimated tickcount %d, correcting\n", plr_index, latency_ms, cmd->tick_count, estimated_tickcount );
                cmd->tick_count = estimated_tickcount;
            }
        }
    }

    if ( original )
        original( thisptr, plr, cmd );
}