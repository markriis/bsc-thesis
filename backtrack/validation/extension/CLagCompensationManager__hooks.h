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

#include <map>
#include <cmath>

#define MAX_TICKCOUNT_DELTA 3

using CBasePlayer__IsBot_t = bool(*)(uintptr_t);

DEFINE_VFTABLE_HOOK(
    CLagCompensationManager__StartLagCompensation,
    void,
    uintptr_t thisptr, uintptr_t plr, CUserCmd* cmd
) {
    CBasePlayer__IsBot_t CBasePlayer__IsBot = (CBasePlayer__IsBot_t)(*(uintptr_t*)(*(uintptr_t*)plr + 3648));

    if ( !CBasePlayer__IsBot( plr ) ) {
        int32_t plr_index = CBasePlayer::GetIndex( plr );

        int32_t cached_cur_cmd_tickcount = g_CurrentTickCmd[ plr_index ];
        int32_t prev_cmd_tickcount = g_PreviousTickCmd[ plr_index ];

        if ( prev_cmd_tickcount > cmd->tick_count ) {
            print_ext(
                "(detection) lag compensation tick count went backwards for player %d! prev=%d cmd=%d cur=%d delta=%d\n",
                plr_index,
                prev_cmd_tickcount, cmd->tick_count,
                cached_cur_cmd_tickcount,
                prev_cmd_tickcount - cmd->tick_count
            );
        }
    }
    

    if ( original )
        original( thisptr, plr, cmd );
}