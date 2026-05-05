#pragma once

#include <cstdarg>
#include <cstdio>
#include "smsdk_ext.h"

void print_ext( const char* fmt, ... ) {
    char message[ 512 ];

    va_list args;
    va_start( args, fmt );
    vsnprintf( message, sizeof( message ), fmt, args );
    va_end( args );

    smutils->LogMessage( myself, "%s", message );
}
