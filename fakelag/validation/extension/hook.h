#pragma once

#include <cstdint>
#include <cstddef>
#include <sys/mman.h>

// from own project: https://github.com/11x1/half-life-2-console-painter/blob/internal-dll/src/hooks/hooks.hh
// apparently linux has no calltypes...?
#define MAKE_VFTABLE_HOOK( hook_name, return_type, ... ) \
namespace all_hooks { \
    namespace hook_name { \
        VirtualMethodHook* vm_hook = nullptr; \
        using def = return_type (  * )( __VA_ARGS__ ); \
        inline def original { nullptr };\
        return_type hook( __VA_ARGS__ );\
    } \
} \
return_type all_hooks::hook_name::hook( __VA_ARGS__ )

#define INITIALIZE_VFTABLE_HOOK( hook_name, vtbl, idx ) { \
    { \
        print_ext( "initializing hook \"%s\" at vtbl=%p\n", #hook_name, (void*)( vtbl ) ); \
        all_hooks::hook_name::vm_hook = new VirtualMethodHook( vtbl, idx ); \
        print_ext( "\torig_func(#%d)=%p\n", idx, (void*)all_hooks::hook_name::vm_hook->original( ) ); \
        all_hooks::hook_name::vm_hook->Hook( reinterpret_cast<uintptr_t>( all_hooks::hook_name::hook ) ); \
    } \
}

#define UNINITIALIZE_VFTABLE_HOOK( hook_name ) { \
    { \
        print_ext( "uninitializing hook \"%s\"\n", #hook_name ); \
        all_hooks::hook_name::vm_hook->Unhook( ); \
        delete all_hooks::hook_name::vm_hook; \
    } \
}


// codex start
static bool MakeWritable(void *addr)
{
    const long pageSize = sysconf(_SC_PAGESIZE);
    const uintptr_t page = reinterpret_cast<uintptr_t>(addr) & ~(static_cast<uintptr_t>(pageSize) - 1);

    if (mprotect(reinterpret_cast<void *>(page), pageSize, PROT_READ | PROT_WRITE) != 0) {
        print_ext("mprotect RW failed addr=%p page=%p errno=%d (%s)",
            addr, reinterpret_cast<void *>(page), errno, strerror(errno));
        return false;
    }

    return true;
}

static bool MakeReadOnly(void *addr)
{
    const long pageSize = sysconf(_SC_PAGESIZE);
    const uintptr_t page = reinterpret_cast<uintptr_t>(addr) & ~(static_cast<uintptr_t>(pageSize) - 1);

    if (mprotect(reinterpret_cast<void *>(page), pageSize, PROT_READ) != 0) {
        print_ext("mprotect R failed addr=%p page=%p errno=%d (%s)",
            addr, reinterpret_cast<void *>(page), errno, strerror(errno));
        return false;
    }

    return true;
}
// codex end

class VirtualMethodHook {
private:
    uintptr_t* m_vtbl;
    int m_index;
    uintptr_t m_original;

public:
    VirtualMethodHook( uintptr_t vtbl, int index )
        : m_vtbl( reinterpret_cast<uintptr_t*>( vtbl ) )
        , m_index( index )
        // , m_original( m_vtbl[ index ] )
        // codex start
        , m_original( 0 )
        // codex end
    {
        // codex start
        print_ext("VirtualMethodHook ctor vtbl=%p index=%d slot=%p",
        reinterpret_cast<void *>(vtbl),
        index,
        reinterpret_cast<void *>(&m_vtbl[index]));

        m_original = m_vtbl[index];

        print_ext("VirtualMethodHook original=%p", reinterpret_cast<void *>(m_original));
        // codex end
    }

    void Hook( uintptr_t func ) {
        // codex start
        void* slot = &m_vtbl[ m_index ];
        print_ext( "Hooking slot #%d at %p (original=%p) with %p\n", m_index, slot, (void*)m_original, (void*)func );
        if ( !MakeWritable( slot ) ) {
            print_ext( "Failed to make slot writable, aborting hook\n" );
            return;
        }
        m_vtbl[ m_index ] = func;
        if ( !MakeReadOnly( slot ) ) {
            print_ext( "Failed to make slot read-only after hooking\n" );
        }
        // codex end

        // mprotect( &m_vtbl[ m_index ], sizeof( uintptr_t ), PROT_READ | PROT_WRITE );
        // m_vtbl[ m_index ] = func;
        // mprotect( &m_vtbl[ m_index ], sizeof( uintptr_t ), PROT_READ );
    }

    void Unhook( ) {
        // codex start
        void* slot = &m_vtbl[ m_index ];
        print_ext( "Unhooking slot #%d at %p, restoring original %p\n", m_index, slot, (void*)m_original );
        if ( !MakeWritable( slot ) ) {
            print_ext( "Failed to make slot writable, aborting unhook\n" );
            return;
        }
        m_vtbl[ m_index ] = m_original;
        if ( !MakeReadOnly( slot ) ) {
            print_ext( "Failed to make slot read-only after unhooking\n" );
        }
        // codex end

        // mprotect( &m_vtbl[ m_index ], sizeof( uintptr_t ), PROT_READ | PROT_WRITE );
        // m_vtbl[ m_index ] = m_original;
        // mprotect( &m_vtbl[ m_index ], sizeof( uintptr_t ), PROT_READ );
    }

    uintptr_t original() const {
        return m_original;
    }
};