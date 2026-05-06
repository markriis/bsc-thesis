#pragma once

#include "smsdk_ext.h"

namespace GameFunctions {
    // IServerUnknown *__fastcall GetContainingEntity(edict_t *a1)
    using GetContainingEntity_t = void* ( * )( edict_t* );
    GetContainingEntity_t GetContainingEntity = nullptr;

    using UTIL_GetPlayerConnectionInfo_t = void (*)( int32_t player_index, int *ping, int *packetloss );
    UTIL_GetPlayerConnectionInfo_t UTIL_GetPlayerConnectionInfo = nullptr;
}