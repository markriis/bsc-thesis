#pragma once

#include <stdint.h>

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