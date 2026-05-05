#include "extension.h"
#include "utils.h"

#include "hook.h"
#include "module_utils.h"
#include "offsets.h"
#include "game_functions.h"
#include "game_interfaces.h"
#include "CBasePlayer__ProcessUserCmds.h"
#include "player_hooks.h"

void HookClient( int client ) {
    IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( client );

    // leave bots alone
    if ( !gamePlayer || gamePlayer->IsFakeClient( ) ) return;

    if ( !gamePlayer->IsConnected( ) || !gamePlayer->IsInGame( ) ) {
        print_ext( "client %d not connected, skipping hook\n", client );
        return;
    }

    edict_t* pPlayerEdict = gamePlayer ? gamePlayer->GetEdict() : nullptr;

    if ( !pPlayerEdict ) {
        print_ext( "failed to get edict for client %d\n", client );
        return;
    }

    // get player base ptr
    uintptr_t player = (uintptr_t)GameFunctions::GetContainingEntity( pPlayerEdict );

    if ( !player ) {
        print_ext( "failed to get player base ptr for client %d\n", client );
        return;
    }

    print_ext( "got player base ptr %p for client %d\n", (void*)player, client );

    // switch ProcessUsercmds to our hook
    g_PlayerHookManager.HookPlayer( client, player, CBasePlayer_ProcessUsercmds_index, (uintptr_t)all_hooks::ProcessUsercmds::hook );
}

void UnhookClient( int client ) {
    IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( client );
    
    // leave bots alone
    if ( !gamePlayer || gamePlayer->IsFakeClient( ) ) return;

    g_PlayerHookManager.UnhookPlayer( client );
}

void CHookHelper::SDK_OnAllLoaded( ) {
    // VirtualMethodHook hook( 0, 0 );
    print_ext( "SDK_OnAllLoaded called, forward created\n" );

    auto server_base = ModuleHelper::FindModuleBase( "server_srv.so" );

    if ( !server_base ) {
        print_ext( "failed to find server base\n" );
        return;
    }

    print_ext( "found server base at %p\n", (void*)server_base );

    GameFunctions::GetContainingEntity = ( GameFunctions::GetContainingEntity_t )( server_base + GetContainingEntity_offset );
    // GameInterfaces::g_pGlobals = ( CGlobalVars* )( server_base + CGlobalVars_offset );

    print_ext( "GetContainingEntity at %p\n", (void*)GameFunctions::GetContainingEntity );

    playerhelpers->AddClientListener( this );

    print_ext( "added client listener\n" );

    // hook every player in server already
    if ( playerhelpers->IsServerActivated( ) ) {
        for ( int i = 1; i <= playerhelpers->GetMaxClients( ); i++ ) {
            IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( i );
            HookClient( i );
        }
    }
}

void CHookHelper::SDK_OnUnload( ) {
    // unhook clients
    for ( int i = 1; i <= playerhelpers->GetMaxClients( ); i++ ) {
        IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( i );
        if ( gamePlayer && gamePlayer->IsConnected( ) && !gamePlayer->IsFakeClient( ) ) {
            UnhookClient( i );
        }
    }

    // unregister client listener
    playerhelpers->RemoveClientListener( this );

    print_ext( "SDK_OnUnload called, hooks released\n" );
}

bool CHookHelper::SDK_OnLoad( char* error, size_t maxlen, bool late ) {
    print_ext( "SDK_OnLoad called, extension loaded\n" );
    return true;
}

void CHookHelper::OnClientPutInServer( int client ) {
    print_ext( "client %d put in server\n", client );

    HookClient( client );
}

void CHookHelper::OnClientDisconnected( int client ) {
    print_ext( "client %d disconnected\n", client );

    UnhookClient( client );
}

void CHookHelper::OnProcessUsercmds_Post( void* plr, CUserCmd* cmd, int numcmds, int totalcmds, int dropped_packets, bool paused ) {
    print_ext( "OnProcessUsercmds_Post called for player %p\n", plr );
    print_ext(
        "OnProcessUsercmds_Post | numcmds=%d, totalcmds=%d, dropped_packets=%d, paused=%d cmdnum=%d\n",
        numcmds, totalcmds, dropped_packets, paused, cmd->command_number
    );
}

SMEXT_LINK(&g_HookHelper);
