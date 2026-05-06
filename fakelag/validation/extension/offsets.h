#pragma once

#define CBasePlayer_vft_offset              0x1945708
#define CBasePlayer_ProcessUsercmds_offset  0x1946480
#define CBasePlayer_ProcessUsercmds_index   ( CBasePlayer_ProcessUsercmds_offset - CBasePlayer_vft_offset ) / sizeof( uintptr_t )

/*
*                                               // confirmed with vft
*   CBasePlayer__PlayerRunCommand = (void (__fastcall *)(CBasePlayer *, void (__fastcall ***)(CUserCmd *__hidden), __int64))*((_QWORD *)this_1 + 432);
*   movehelper_server = MoveHelperServer();
*   v81 = &vecAvailCommands;
*   CBasePlayer__PlayerRunCommand(this, &vecAvailCommands, movehelper_server);
*   from ida
*   void __fastcall CBasePlayer::PhysicsSimulate(CBasePlayer *this, double a2, __int64 *a3, __int64 a4)

*   needed to see what commands the server runs
*/
#define CBasePlayer_PlayerRunCommand_index 432

#define GetContainingEntity_offset          0x983450

#define CGlobalVars_offset                  0x1BD86A0