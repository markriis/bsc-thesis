#pragma once

#include <stdint.h>
#include <map>
#include "sdk_defines.h"

// * player index -> last cmd processed for that player
// * used to track when cmd count changes in lag compensation
static auto g_PreviousTickCmd = std::map< int32_t, int32_t >( );
static auto g_CurrentTickCmd = std::map< int32_t, int32_t >( );

namespace CBasePlayer {
    int32_t GetIndex( uintptr_t p_plr ) {
        int32_t plr_index = 0;
        auto v10 = *reinterpret_cast< uintptr_t* >( p_plr + 8 );
        if ( v10 )
            // movsx   esi, word ptr [rax+6]
            plr_index = *reinterpret_cast< int16_t* >( v10 + 6 );
        return plr_index;
    }
}