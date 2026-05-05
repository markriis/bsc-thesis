#pragma once

#include <map>
#include <memory>
#include "hook.h"
#include "utils.h"

// TODO: vfts can be shared :facepalm:, fix! 
class PlayerHookManager {
private:
    std::map< int, std::map< int, std::unique_ptr< VirtualMethodHook > > > m_hooks;

public:
    PlayerHookManager() = default;

    void HookPlayer( int client, uintptr_t playerBase, int index, uintptr_t hookFunc ) {
        // vtbl always first
        uintptr_t vtbl = *reinterpret_cast< uintptr_t* >( playerBase );

        print_ext( "hooking client %d playerBase=%p vft=%p index=%d hookFunc=%p\n", client, (void*)playerBase, (void*)vtbl, index, (void*)hookFunc );

        auto hook = std::make_unique< VirtualMethodHook >( vtbl, index );
        
        hook->Hook( hookFunc );
        
        // store hook at client idx and method index
        m_hooks[ client ][ index ] = std::move( hook );
    }

    void UnhookPlayer( int client, int index ) {
        // find client hook
        auto client_map = m_hooks.find( client );
        if ( client_map != m_hooks.end() ) {

            // look for hook for method index
            auto& hooks = client_map->second;
            auto hook = hooks.find( index );
            if ( hook != hooks.end() ) {
                hook->second->Unhook();
                hooks.erase( hook );
            }

            // clear map if no hooks left for client
            if ( hooks.empty() ) {
                m_hooks.erase( client_map );
            }
        }
    }

    void UnhookPlayer( int client ) {
        auto client_map = m_hooks.find( client );

        // unhook EVERYTHINGGGGGGGGG
        if ( client_map != m_hooks.end( ) ) {
            for ( auto& [ index, hook ] : client_map->second ) {
                hook->Unhook();
            }
            m_hooks.erase( client_map );
        }
    }
};

inline PlayerHookManager g_PlayerHookManager;