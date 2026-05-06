#include "extension.h"
#include "utils.h"

#include "hook.h"
#include "module_utils.h"
#include "offsets.h"
#include "game_functions.h"
#include "game_interfaces.h"
#include "CBasePlayer__PlayerRunCommand.h"
#include "CLagCompensationManager__hooks.h"
#include "player_hooks.h"

void HookClient( int client ) {
    IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( client );

    // leave bots alone
    if ( !gamePlayer || gamePlayer->IsFakeClient( ) ) return;

    // uhh is this actually needed
    if ( !gamePlayer->IsConnected( ) || !gamePlayer->IsInGame( ) ) {
        print_ext_scoped( "client %d not connected, skipping hook\n", client );
        return;
    }

    edict_t* pPlayerEdict = gamePlayer ? gamePlayer->GetEdict() : nullptr;

    if ( !pPlayerEdict ) {
        print_ext_scoped( "failed to get edict for client %d\n", client );
        return;
    }

    // get player base ptr
    uintptr_t player = (uintptr_t)GameFunctions::GetContainingEntity( pPlayerEdict );

    if ( !player ) {
        print_ext_scoped( "failed to get player base ptr for client %d\n", client );
        return;
    }

    print_ext_scoped( "got player base ptr %p for client %d\n", (void*)player, client );

    // switch ProcessUsercmds to our hook
    g_PlayerHookManager.HookPlayer( client, player, CBasePlayer_PlayerRunCommand_index, (uintptr_t)VFuncHooks::PlayerRunCommand::hook );
}

// todo: could be cleaned up as function above needs playebase aswell
void UnhookClient( int client ) {
    IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( client );
    
    // leave bots alone
    // lol bots arent fake clients
    if ( !gamePlayer || gamePlayer->IsFakeClient( ) ) return;

    edict_t* pPlayerEdict = gamePlayer ? gamePlayer->GetEdict() : nullptr;

    if ( !pPlayerEdict ) {
        print_ext_scoped( "failed to get edict for client %d\n", client );
        return;
    }

    // get player base ptr
    uintptr_t player = (uintptr_t)GameFunctions::GetContainingEntity( pPlayerEdict );

    if ( !player ) {
        print_ext_scoped( "failed to get player base ptr for client %d\n", client );
        return;
    }

    g_PlayerHookManager.UnhookPlayerAll( client, player );
}

void CHookHelper::SDK_OnAllLoaded( ) {
    // VirtualMethodHook hook( 0, 0 );
    print_ext_scoped( "SDK_OnAllLoaded called, forward created\n" );

    auto server_base = ModuleHelper::FindModuleBase( "server_srv.so" );

    if ( !server_base ) {
        print_ext_scoped( "failed to find server base\n" );
        return;
    }

    print_ext_scoped( "found server base at %p\n", (void*)server_base );

    GameFunctions::GetContainingEntity = ( GameFunctions::GetContainingEntity_t )( server_base + GetContainingEntity_offset );
    GameInterfaces::g_pGlobals = *( CGlobalVars** )( server_base + CGlobalVars_offset );
    GameInterfaces::g_pEngineServer = *( CEngineServer** )( server_base + CEngineServer_offset );
    // todo: maybe double deref, changed before i went to coffee
    GameInterfaces::g_pLagCompensationManager = *( CLagCompensationManager** )( server_base + CLagCompensationManager_offset );

    print_ext_scoped( "game specific offsets initialized\n" );
    print_ext_scoped( "\t GetContainingEntity: %p\n", (void*)GameFunctions::GetContainingEntity );
    print_ext_scoped( "\t CGlobalVars: %p\n", (void*)GameInterfaces::g_pGlobals );
    print_ext_scoped( "\t CEngineServer: %p\n", (void*)GameInterfaces::g_pEngineServer );
    print_ext_scoped( "\t CLagCompensationManager: %p\n", (void*)GameInterfaces::g_pLagCompensationManager );

    playerhelpers->AddClientListener( this );

    // hook every player in server already
    if ( playerhelpers->IsServerActivated( ) ) {
        for ( int i = 1; i <= playerhelpers->GetMaxClients( ); i++ ) {
            IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( i );
            HookClient( i );
        }
    }

    // hook lc
    VFuncHooks::CLagCompensationManager__StartLagCompensation::original = 
        (VFuncHooks::CLagCompensationManager__StartLagCompensation::def)
        VirtualMethodHelper::Hook(
            *(uintptr_t**)GameInterfaces::g_pLagCompensationManager,
            0,
            (uintptr_t)VFuncHooks::CLagCompensationManager__StartLagCompensation::hook
        );
    
    print_ext_scoped( "hooks initialized\n" );
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

    print_ext_scoped( "hooks released\n" );
}

bool CHookHelper::SDK_OnLoad( char* error, size_t maxlen, bool late ) {
    print_ext_scoped( "extension loaded\n" );
    return true;
}

void CHookHelper::OnClientPutInServer( int client ) {
    print_ext_scoped( "client %d put in server\n", client );

    HookClient( client );
}

void CHookHelper::OnClientDisconnected( int client ) {
    print_ext_scoped( "client %d disconnected\n", client );

    UnhookClient( client );
}

SMEXT_LINK(&g_HookHelper);
