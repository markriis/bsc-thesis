#include "extension.h"
#include "utils.h"

#include "hook.h"
#include "module_utils.h"
#include "offsets.h"
#include "game_interfaces.h"
#include "CLagCompensationManager__hooks.h"


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

    print_ext_scoped( "game specific offsets initialized\n" );
    print_ext_scoped( "\t CGlobalVars: %p\n", (void*)GameInterfaces::g_pGlobals );
    print_ext_scoped( "\t CEngineServer: %p\n", (void*)GameInterfaces::g_pEngineServer );
    print_ext_scoped( "\t CLagCompensationManager: %p\n", (void*)GameInterfaces::g_pLagCompensationManager );

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

// todo: well something goes wrong here, upon unloading the extension
// *     core gets dumped, #uncool
void CHookHelper::SDK_OnUnload( ) {
    VirtualMethodHelper::Unhook( *(uintptr_t**)GameInterfaces::g_pLagCompensationManager, 0, (uintptr_t)VFuncHooks::CLagCompensationManager__StartLagCompensation::original );

    print_ext_scoped( "hook released\n" );
}

bool CHookHelper::SDK_OnLoad( char* error, size_t maxlen, bool late ) {
    print_ext_scoped( "extension loaded\n" );
    return true;
}

SMEXT_LINK(&g_HookHelper);
