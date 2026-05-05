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

void print_ext( const char* funcname_macro, int linenumber_macro, const char* fmt, ... ) {
    char message[ 2048 ];

    va_list args;
    va_start( args, fmt );
    vsnprintf( message, sizeof( message ), fmt, args );
    va_end( args );

    smutils->LogMessage( myself, "[%s:%d] %s", funcname_macro, linenumber_macro, message );
}

#define print_ext_scoped( fmt, ... ) print_ext( __func__, __LINE__, fmt, ##__VA_ARGS__ )