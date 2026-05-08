#include "extension.h"
#include "utils.h"

#include "hook.h"
#include "module_utils.h"
#include "offsets.h"
#include "game_interfaces.h"
#include "game_functions.h"
#include "player_hooks.h"

#include "CBasePlayer__PlayerRunCommand.h"
#include "CLagCompensationManager__StartLagCompensation.h"

uintptr_t GetCBasePlayerFromClient( int client ) {
    IGamePlayer* gamePlayer = playerhelpers->GetGamePlayer( client );

    if ( !gamePlayer || gamePlayer->IsFakeClient( ) ) return 0;

    if ( !gamePlayer->IsConnected( ) || !gamePlayer->IsInGame( ) ) {
        return 0;
    }

    edict_t* pPlayerEdict = gamePlayer ? gamePlayer->GetEdict() : nullptr;

    if ( !pPlayerEdict ) return 0;

    // get player base ptr
    uintptr_t player = (uintptr_t)GameFunctions::GetContainingEntity( pPlayerEdict );

    return player;
}

void HookClient( int client ) {
    // get player base ptr
    uintptr_t player = GetCBasePlayerFromClient( client );

    if ( !player ) {
        print_ext_scoped( "failed to get player base ptr for client %d\n", client );
        return;
    }

    print_ext_scoped( "got player base ptr %p for client %d\n", (void*)player, client );

    // switch ProcessUsercmds to our hook
    g_PlayerHookManager.HookPlayer( client, player, CBasePlayer_PlayerRunCommand_index, (uintptr_t)VFuncHooks::PlayerRunCommand::hook );
}

void UnhookClient( int client ) {
    uintptr_t player = GetCBasePlayerFromClient( client );

    if ( !player ) {
        print_ext_scoped( "failed to get player base ptr for client %d\n", client );
        return;
    }

    g_PlayerHookManager.UnhookPlayerAll( client, player );
}

void CHookHelper::SDK_OnAllLoaded( ) {
    print_ext_scoped( "SDK_OnAllLoaded called, forward created\n" );

    auto server_base = ModuleHelper::FindModuleBase( "server_srv.so" );

    if ( !server_base ) {
        print_ext_scoped( "failed to find server base\n" );
        return;
    }

    print_ext_scoped( "found server base at %p\n", (void*)server_base );

    GameInterfaces::g_pGlobals = *( CGlobalVars** )( server_base + CGlobalVars_offset );
    GameInterfaces::g_pEngineServer = *( CEngineServer** )( server_base + CEngineServer_offset );
    GameInterfaces::g_pLagCompensationManager = *( CLagCompensationManager** )( server_base + CLagCompensationManager_offset );
    GameFunctions::GetContainingEntity = (GameFunctions::GetContainingEntity_t)( server_base + GetContainingEntity_offset );

    print_ext_scoped( "game specific offsets initialized\n" );

    // hook lc
    VFuncHooks::StartLagCompensation::original = 
        (VFuncHooks::StartLagCompensation::def)
        VirtualMethodHelper::Hook(
            *(uintptr_t**)GameInterfaces::g_pLagCompensationManager,
            0,
            (uintptr_t)VFuncHooks::StartLagCompensation::hook
        );
    
    // hook connected clients
    playerhelpers->AddClientListener( this );
    if ( playerhelpers->IsServerActivated( ) ) {
        for ( int i = 1; i <= playerhelpers->GetMaxClients( ); i++ )
            HookClient( i );
    }
    
    print_ext_scoped( "hooks initialized\n" );
}

void CHookHelper::SDK_OnUnload( ) {
    VirtualMethodHelper::Unhook( *(uintptr_t**)GameInterfaces::g_pLagCompensationManager, 0, (uintptr_t)VFuncHooks::StartLagCompensation::original );

    playerhelpers->RemoveClientListener( this );

    // * unhook connected clients
    if ( playerhelpers->IsServerActivated( ) ) {
        for ( int i = 1; i <= playerhelpers->GetMaxClients( ); i++ )
            UnhookClient( i );
    }

    print_ext_scoped( "hooks released\n" );
}

bool CHookHelper::SDK_OnLoad( char* error, size_t maxlen, bool late ) {
    print_ext_scoped( "extension loaded\n" );
    return true;
}

void CHookHelper::OnClientPutInServer( int client ) {
    HookClient( client );
}

void CHookHelper::OnClientDisconnected( int client ) {
    UnhookClient( client );
}

SMEXT_LINK(&g_HookHelper);
