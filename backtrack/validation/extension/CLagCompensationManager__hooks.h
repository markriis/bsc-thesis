#pragma once

/*
    * StartLagCompensation is a vft func, that's fine to hook (2 variations though, one is a non-virtual thunk)
CLagCompensationManager *__fastcall CLagCompensationManager::FinishLagCompensation(
        CLagCompensationManager *this,
        Vector *a2,
        _QWORD *a3)
{

and

void __fastcall `non-virtual thunk to'CLagCompensationManager::StartLagCompensation(
        CLagCompensationManager *this,
        CBasePlayer *a2,
        CUserCmd *a3)
{
  CLagCompensationManager::StartLagCompensation((CLagCompensationManager *)((char *)this - 24), a2, a3);
}

*(pPlayer+4368) -> active cmd

vvv --- index 18
* .data.rel.ro:000000000189AAB0 30 31 A4 00 00 00 00 00                 dq offset _ZN23CLagCompensationManager20StartLagCompensationEP11CBasePlayerP8CUserCmd ; CLagCompensationManager::StartLagCompensation(CBasePlayer *,CUserCmd *)
  .data.rel.ro:000000000189AAB8 70 07 A4 00 00 00 00 00                 dq offset _ZN23CLagCompensationManager21FinishLagCompensationEP11CBasePlayer ; CLagCompensationManager::FinishLagCompensation(CBasePlayer *)
  .data.rel.ro:000000000189AAC0 00 3E A4 00 00 00 00 00                 dq offset _ZNK23CLagCompensationManager31IsCurrentlyDoingLagCompensationEv ; CLagCompensationManager::IsCurrentlyDoingLagCompensation(void)
  .data.rel.ro:000000000189AAC8 E8 FF FF FF FF FF FF FF                 dq -24                  ; offset to this
  .data.rel.ro:000000000189AAD0 68 85 94 01 00 00 00 00                 dq offset _ZTI23CLagCompensationManager ; `typeinfo for'CLagCompensationManager
* .data.rel.ro:000000000189AAD8 A0 35 A4 00 00 00 00 00                 dq offset _ZThn24_N23CLagCompensationManager20StartLagCompensationEP11CBasePlayerP8CUserCmd
^^^ --- index 23
need to think how I should do this :p

* maybe via FX_FireBullets? I'm thinking, that since there was no network info when
*   engine method called from PlayerRunCommand, the information might be available
*   later on, eg in StartLagCompensation?
*
* so if we go a few layers up (FX_Firebullets->CLagCompensationManager::StartLagCompensation)
*   it might be populated there?

further reversing of FX_FireBullets shows that StartLagCompensation is thiscalled
~ TE_FireBullets(v19, (const Vector *)a3, a4, iWeaponID, a6, a9, a7, a10);
~ CTFPlayer::NoteWeaponFired((CTFPlayer *)pPlayer);
~ lagcompensation->vft->StartLagCompensation(
~    lagcompensation,
~    (CBasePlayer *)pPlayer,
~    *(CUserCmd **)(pPlayer + 4368));
~ if ( g_pPasstimeLogic && CTFPasstimeLogic::GetBall((CTFPasstimeLogic *)g_pPasstimeLogic) )
weirdly enough, StartLagCompensation is the first func in the vft???
*/

#include "hook.h"
#include "sdk_defines.h"
#include "utils.h"
#include "player_hooks.h"
#include "offsets.h"
#include "game_interfaces.h"
#include "extension.h"

#include <map>
#include <cmath>

#define MAX_TICKCOUNT_DELTA 4

using CBasePlayer__IsBot_t = bool(*)(uintptr_t);

// todo: nb_stop 0 made it get called 247
// bot_mirror / bot_mimic
DEFINE_VFTABLE_HOOK(
    CLagCompensationManager__StartLagCompensation,
    void,
    uintptr_t thisptr, uintptr_t plr, CUserCmd* cmd
) {
    // from void __fastcall CPlayerInfo::SetAbsAngles(CPlayerInfo *this, QAngle *ang)
    // v4 = *(__int64 (__fastcall **)(CBasePlayer *__hidden))(*(_QWORD *)m_pParent + 3648LL);
    // v4 -> CBasePlayer::IsBot
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
        interval_per_tick = gpGlobals->interval_per_tick__0x1C;
        */
       
        int32_t plr_index = 0;
        auto v10 = *reinterpret_cast< uintptr_t* >( thisptr + 8 );
        if ( v10 )
            // movsx   esi, word ptr [rax+6]
            // not quite sure if this is the correct way to do it...
            plr_index = *reinterpret_cast< int16_t* >( v10 + 6 ) + 1;
        
        
        auto nci = (CNetChannel*)GameInterfaces::g_pEngineServer->vft->GetPlayerNetInfo(GameInterfaces::g_pEngineServer, plr_index);

        bool found_nci_test = false;
        for ( int i = 0; i < 100; i++ ) {
            auto nci_test = (CNetChannel*)GameInterfaces::g_pEngineServer->vft->GetPlayerNetInfo(GameInterfaces::g_pEngineServer, i);
            if ( nci_test ) {
                found_nci_test = true;
                print_ext_scoped( "got net channel info for player index %d\n", i );
                break;
            }
        }

        if ( !found_nci_test ) {
            print_ext_scoped( "failed to get any netchannelinfos 0..99\n" );
        }

        if ( nci ) {
            float plr_latency = nci->vft->GetLatency((INetChannelInfo *)nci, FLOW_OUTGOING);

            int32_t plr_latency_ticks = TIME_TO_TICKS( plr_latency );
            int32_t estimated_tickcount = GameInterfaces::g_pGlobals->tickcount - plr_latency_ticks;

            // * std::abs was yelling at me
            int32_t diff = estimated_tickcount > cmd->tick_count
                    ? estimated_tickcount - cmd->tick_count
                    : cmd->tick_count - estimated_tickcount;

            if ( diff > MAX_TICKCOUNT_DELTA ) {
                int latency_ms = static_cast<int>(plr_latency * 1000);
                print_ext_scoped( "client %d (%dms) cmd tickcount %d is too far from estimated tickcount %d, correcting\n", plr_index, latency_ms, cmd->tick_count, estimated_tickcount );
                cmd->tick_count = estimated_tickcount;
            }
        } else {
            // todo: fix getting ping, something is broken with getting player netinfo
            int32_t diff = cmd->tick_count > GameInterfaces::g_pGlobals->tickcount
                    ? cmd->tick_count - GameInterfaces::g_pGlobals->tickcount
                    : GameInterfaces::g_pGlobals->tickcount - cmd->tick_count;
            
            if ( diff > MAX_TICKCOUNT_DELTA ) {
                print_ext_scoped( "heavy fallback, client %d cmd tickcount %d is too far from server tickcount %d, correcting\n", plr_index, cmd->tick_count, GameInterfaces::g_pGlobals->tickcount );
                cmd->tick_count = GameInterfaces::g_pGlobals->tickcount;
            } else {
                print_ext_scoped( "client %d has no net channel info, but cmd tickcount %d is close to server tickcount %d, not correcting\n", plr_index, cmd->tick_count, GameInterfaces::g_pGlobals->tickcount );
            }
        }
    }
     

    if ( original )
        original( thisptr, plr, cmd );
}