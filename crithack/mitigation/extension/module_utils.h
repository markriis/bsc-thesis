#pragma once

#include <cstdint>
#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SearchResult {
    const char* name;
    uintptr_t base = 0;
};

static int dl_iterate_callback( struct dl_phdr_info* info, size_t size, void* data ) {
    SearchResult* search_result = static_cast< SearchResult* >( data );
    const char* name = search_result->name;

    if ( strstr( info->dlpi_name, name ) ) {
        search_result->base = info->dlpi_addr;
        return 1;
    }
    return 0;
}

namespace ModuleHelper {    
    static uintptr_t FindModuleBase( const char* name ) {
        SearchResult search;
        search.name = name;

        // https://www.man7.org/linux/man-pages/man3/dl_iterate_phdr.3.html
        dl_iterate_phdr( dl_iterate_callback, &search );
        return search.base;
    };
};