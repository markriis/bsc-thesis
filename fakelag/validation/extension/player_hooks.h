#pragma once

#include <map>
#include <set>
#include <memory>
#include "hook.h"
#include "utils.h"

// TODO: vfts can be shared :facepalm:, fix! 

/*
* idea:
*   keep a list of clients and one global map for vft index -> original
*   when adding a hook:
*   - check if indexed method is set to original (if exists)
*   - if not, create hook and store original in global map
*   - if yes, just create hook
*   when removing a hook:
*   - remove hook and replace with stored original
*   - if no more hooks for that index, remove original from global map
*/
class PlayerHookManager {
private:
    // client -> hooked indexes
    std::map< int, std::set< int > > hooked_clients;

    // hooked index -> original function
    std::map< int, uintptr_t > original_methods; // index -> original

private:
    void AddClient( int client, int method_index ) {
        bool has_client = hooked_clients.find( client ) != hooked_clients.end( );

        if ( !has_client ) {
            // no client, new set
            hooked_clients[ client ] = std::set< int >{ method_index };
        } else {
            // oop we exist
            hooked_clients[ client ].insert( method_index );
        }
    }

    void RemoveClient( int client, int method_index ) {
        auto it = hooked_clients.find( client );

        if ( it == hooked_clients.end( ) ) return;

        // remove method index from set
        it->second.erase( method_index );

        // if set is empty, remove client from map
        if ( it->second.empty( ) ) {
            hooked_clients.erase( it );
        }
    }

    void SetOriginal( int index, uintptr_t original ) {
        original_methods[ index ] = original;
    }
public:
    PlayerHookManager() = default;

    void HookPlayer( int client, uintptr_t playerBase, int index, uintptr_t hookFunc ) {
        // vtbl always first
        uintptr_t* vtbl = *reinterpret_cast< uintptr_t** >( playerBase );

        print_ext( "hooking client %d playerBase=%p vft=%p index=%d hookFunc=%p\n", client, (void*)playerBase, (void*)vtbl, index, (void*)hookFunc );
        
        auto vtbl_fn = VirtualMethodHelper::GetVtblMethodAddress( vtbl, index );
        auto cached_original_fn = GetOriginal( index );

        // not hooked, go ahead
        if ( !cached_original_fn ) {
            // note: would do this after checking if we're rehooking,
            //       but that way if original isn't set, we never hook 
            auto original_fn = VirtualMethodHelper::Hook( vtbl, index, hookFunc );
            
            SetOriginal( index, original_fn );
            AddClient( client, index );
            
            return;
        }

        // already have original
        // check if we're rehooking, only allow hooking if original matches cached original
        // otherwise something is very wrong
        if ( cached_original_fn != vtbl_fn ) {
            print_ext( "WARNING: original method for index %d does not match cached original! cached=%p actual=%p\n", index, (void*)cached_original_fn, (void*)vtbl_fn );
            return;
        }
        
        // original matches stored original, safe to hook 
        VirtualMethodHelper::Hook( vtbl, index, hookFunc );

        AddClient( client, index );
    }

    void UnhookPlayer( int client, uintptr_t playerBase, int index ) {
        // have we hooked this client before?
        auto hooked_client = hooked_clients.find( client );
        
        // player wasn't hooked, nothing to do
        if ( hooked_client == hooked_clients.end( ) ) return;

        uintptr_t* vtbl = *reinterpret_cast< uintptr_t** >( playerBase );
        auto vtbl_fn = VirtualMethodHelper::GetVtblMethodAddress( vtbl, index );
        auto cached_original_fn = GetOriginal( index );

        // can we restore original?
        if ( !cached_original_fn ) {
            print_ext( "client %d index %d not hooked, nothing to unhook\n", client, index );
            return;
        }

        // unhookable if current vtbl doesn't match cached original
        if ( cached_original_fn == vtbl_fn ) {
            print_ext( "WARNING: cannot unhook client %d index %d, vtbl method matches cached original\n", client, index );
            return;
        }

        // safe to unhook
        VirtualMethodHelper::Unhook( vtbl, index, cached_original_fn );

        // remove ourselves from hooked clients list
        RemoveClient( client, index );

        // check if any other clients are hooked with this original, if not remove from map
        bool still_hooked = false;
        for ( auto& [ other_client, indexes ] : hooked_clients ) {
            if ( other_client == client ) continue; // skip self

            // does the other client have this index hooked?
            if ( indexes.find( index ) != indexes.end( ) ) {
                still_hooked = true;
                break;
            }
        }

        // ok clear to remove original
        if ( !still_hooked ) {
            original_methods.erase( index );
        }
    }

    void UnhookPlayerAll( int client, uintptr_t playerBase ) {
        // have we hooked this client before?
        auto hooked_client = hooked_clients.find( client );

        if ( hooked_client == hooked_clients.end( ) ) return;

        // unhook alles
        for ( auto& index : hooked_client->second ) {
            UnhookPlayer( client, playerBase, index );
        }
    }

    // get original for method index, if exists
    // would be private but hooks need access to call original so public it is
    template <typename T = uintptr_t >
    T GetOriginal( int vft_index ) {
        auto it = original_methods.find( vft_index );
        // do we have the original
        if ( it != original_methods.end() )
            return reinterpret_cast< T >( it->second );
        
        // otherwise null
        return 0;
    }
};

inline PlayerHookManager g_PlayerHookManager;