#pragma once

#include "sdk_defines.h"
#include "stdint.h"

namespace CBasePlayer {
    int32_t GetIndex( uintptr_t thisptr ) {
        int32_t plr_index = 0;
        auto v10 = *reinterpret_cast< uintptr_t* >( thisptr + 8 );
        if ( v10 )
            // movsx   esi, word ptr [rax+6]
            plr_index = *reinterpret_cast< int16_t* >( v10 + 6 );
        return plr_index;
    }
}


void CopyUserCmd( CUserCmd* dest, CUserCmd* src ) {
    dest->command_number = src->command_number;
    dest->tick_count = src->tick_count;
    dest->viewangles = src->viewangles;
    dest->forwardmove = src->forwardmove;
    dest->sidemove = src->sidemove;
    dest->upmove = src->upmove;
    dest->buttons = src->buttons;
    dest->impulse = src->impulse;
    dest->weaponselect = src->weaponselect;
    dest->weaponsubtype = src->weaponsubtype;
    dest->random_seed = src->random_seed;
    dest->server_random_seed = src->server_random_seed;
    dest->mousedx = src->mousedx;
    dest->mousedy = src->mousedy;
    dest->hasbeenpredicted = src->hasbeenpredicted;
}