#pragma once

#include <stdint.h>
#include <map>
#include "sdk_defines.h"

// * player index -> last cmd processed for that player
// * used to track when cmd count changes in lag compensation
static auto g_LastSimulatedCmd = std::map< int32_t, CUserCmd >( );
