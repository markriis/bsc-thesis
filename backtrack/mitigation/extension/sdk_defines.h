#pragma once

// * contains reversed sdk structs and defines
// *   used for hooking and interfacing with the game
// *   types commented with SDK-<path>/SDK-end are from the source sdk
// *   and haven't been verified by decomp 

// SDK-game/shared/in_buttons.h
#define IN_ATTACK		(1 << 0)
#define IN_JUMP			(1 << 1)
#define IN_DUCK			(1 << 2)
#define IN_FORWARD		(1 << 3)
#define IN_BACK			(1 << 4)
#define IN_USE			(1 << 5)
#define IN_CANCEL		(1 << 6)
#define IN_LEFT			(1 << 7)
#define IN_RIGHT		(1 << 8)
#define IN_MOVELEFT		(1 << 9)
#define IN_MOVERIGHT	(1 << 10)
#define IN_ATTACK2		(1 << 11)
#define IN_RUN			(1 << 12)
#define IN_RELOAD		(1 << 13)
#define IN_ALT1			(1 << 14)
#define IN_ALT2			(1 << 15)
#define IN_SCORE		(1 << 16)   // Used by client.dll for when scoreboard is held down
#define IN_SPEED		(1 << 17)	// Player is holding the speed key
#define IN_WALK			(1 << 18)	// Player holding walk key
#define IN_ZOOM			(1 << 19)	// Zoom key for HUD zoom
#define IN_WEAPON1		(1 << 20)	// weapon defines these bits
#define IN_WEAPON2		(1 << 21)	// weapon defines these bits
#define IN_BULLRUSH		(1 << 22)
#define IN_GRENADE1		(1 << 23)	// grenade 1
#define IN_GRENADE2		(1 << 24)	// grenade 2
#define	IN_ATTACK3		(1 << 25)
// SDK-end

typedef unsigned char byte;

typedef float float32_t;
typedef double float64_t;

struct bf_write
{
  unsigned int *m_pData;
  int m_nDataBytes;
  int m_nDataBits;
  int m_iCurBit;
  bool m_bOverflow;
  bool m_bAssertOnOverflow;
  uint8_t pad_0[2];
  const char *m_pDebugName;
};

template <typename T>
struct CUtlMemory
{
  T *m_pMemory;
  int m_nAllocationCount;
  int m_nGrowSize;
};

template <typename T>
struct CUtlVector
{
  CUtlMemory< T > m_Memory;
  int m_Size;
  uint8_t _pad0[4];
  uint8_t *m_pElements;
};

struct Vector {
    float x, y, z;
};

struct QAngle {
    float x, y, z;
};

// * only reason we have a vft is for the desctructor :rofl: 
struct CUserCmd {
  void *vft;
  int32_t command_number;
  uint32_t tick_count;
  QAngle viewangles;
  float32_t forwardmove;
  float32_t sidemove;
  float32_t upmove;
  int32_t buttons;
  int8_t impulse;
  int32_t weaponselect;
  int32_t weaponsubtype;
  int32_t random_seed;
  int32_t server_random_seed;
  uint16_t mousedx;
  uint16_t mousedy;
  bool hasbeenpredicted;
};



struct CGlobalVars {
  uint8_t pax_0x0[24];
  int32_t tickcount;
  float interval_per_tick__0x1C;
};

struct INetChannelInfo { };
struct INetChannelInfo_vft
{
  const char *(*GetName)(INetChannelInfo *thisptr);
  const char *(*GetAddress)(INetChannelInfo *thisptr);
  float (*GetTime)(INetChannelInfo *thisptr);
  float (*GetTimeConnected)(INetChannelInfo *thisptr);
  int (*GetBufferSize)(INetChannelInfo *thisptr);
  int (*GetDataRate)(INetChannelInfo *thisptr);
  bool (*IsLoopback)(INetChannelInfo *thisptr);
  bool (*IsTimingOut)(INetChannelInfo *thisptr);
  bool (*IsPlayback)(INetChannelInfo *thisptr);
  float (*GetLatency)(INetChannelInfo *thisptr, int flow);
  float (*GetAvgLatency)(INetChannelInfo *thisptr, int flow);
  float (*GetAvgLoss)(INetChannelInfo *thisptr, int flow);
  float (*GetAvgChoke)(INetChannelInfo *thisptr, int flow);
  float (*GetAvgData)(INetChannelInfo *thisptr, int flow);
  float (*GetAvgPackets)(INetChannelInfo *thisptr, int flow);
  int (*GetTotalData)(INetChannelInfo *thisptr, int flow);
  int (*GetSequenceNr)(INetChannelInfo *thisptr, int flow);
  bool (*IsValidPacket)(INetChannelInfo *thisptr, int flow, int frame_number);
  float (*GetPacketTime)(INetChannelInfo *thisptr, int flow, int frame_number);
  int (*GetPacketBytes)(INetChannelInfo *thisptr, int flow, int frame_number, int group);
  bool (*GetStreamProgress)(INetChannelInfo *thisptr, int flow, int *received, int *total);
  float (*GetTimeSinceLastReceived)(INetChannelInfo *thisptr);
  float (*GetCommandInterpolationAmount)(INetChannelInfo *thisptr, int flow, int frame_number);
  void (*GetPacketResponseLatency)(INetChannelInfo *thisptr, int flow, int frame_number, int *pnLatencyMsecs, int *pnChoke);
  void (*GetRemoteFramerate)(INetChannelInfo *thisptr, float *pflFrameTime, float *pflFrameTimeStdDeviation);
  float (*GetTimeoutSeconds)(INetChannelInfo *thisptr);
};

struct INetChannel { };
struct INetMessage { };
struct ConnectionStatus_t { };

struct INetChannel_vft : INetChannelInfo_vft
{
  void *(*destructor)(INetChannel *thisptr, unsigned int flags);
  void (*SetDataRate)(INetChannel *thisptr, float rate);
  bool (*RegisterMessage)(INetChannel *thisptr, void *msg);
  void (*SetTimeout)(INetChannel *thisptr, float seconds);
  void (*SetDemoRecorder)(INetChannel *thisptr, void *recorder);
  void (*SetChallengeNr)(INetChannel *thisptr, unsigned int chnr);
  void (*Reset)(INetChannel *thisptr);
  void (*Clear)(INetChannel *thisptr);
  void (*Shutdown)(INetChannel *thisptr, const char *reason);
  void (*ProcessPlayback)(INetChannel *thisptr);
  void (*ProcessPacket)(INetChannel *thisptr, void *packet, bool bHasHeader);
  bool (*SendNetMsg)(INetChannel *thisptr, INetMessage *msg, bool bForceReliable, bool bVoice);
  bool (*SendData)(INetChannel *thisptr, bf_write *msg, bool bReliable);
  bool (*SendFile)(INetChannel *thisptr, const char *filename, unsigned int transferID);
  void (*DenyFile)(INetChannel *thisptr, const char *filename, unsigned int transferID);
  void (*RequestFile_OLD)(INetChannel *thisptr, const char *filename, unsigned int transferID);
  void (*SetChoked)(INetChannel *thisptr);
  int (*SendDatagram)(INetChannel *thisptr, bf_write *data);
  bool (*Transmit)(INetChannel *thisptr, bool onlyReliable);
  const void *(*GetRemoteAddress)(INetChannel *thisptr);
  void *(*GetMsgHandler)(INetChannel *thisptr);
  int (*GetDropNumber)(INetChannel *thisptr);
  int (*GetSocket)(INetChannel *thisptr);
  unsigned int (*GetChallengeNr)(INetChannel *thisptr);
  void (*GetSequenceData)(INetChannel *thisptr, int *nOutSequenceNr, int *nInSequenceNr, int *nOutSequenceNrAck);
  void (*SetSequenceData)(INetChannel *thisptr, int nOutSequenceNr, int nInSequenceNr, int nOutSequenceNrAck);
  void (*UpdateMessageStats)(INetChannel *thisptr, int msggroup, int bits);
  bool (*CanPacket)(INetChannel *thisptr);
  bool (*IsOverflowed)(INetChannel *thisptr);
  bool (*IsTimedOut)(INetChannel *thisptr);
  bool (*HasPendingReliableData)(INetChannel *thisptr);
  void (*SetFileTransmissionMode)(INetChannel *thisptr, bool bBackgroundMode);
  void (*SetCompressionMode)(INetChannel *thisptr, bool bUseCompression);
  unsigned int (*RequestFile)(INetChannel *thisptr, const char *filename);
  void (*SetMaxBufferSize)(INetChannel *thisptr, bool bReliable, int nBytes, bool bVoice);
  bool (*IsNull)(INetChannel *thisptr);
  int (*GetNumBitsWritten)(INetChannel *thisptr, bool bReliable);
  void (*SetInterpolationAmount)(INetChannel *thisptr, float flInterpolationAmount);
  void (*SetRemoteFramerate)(INetChannel *thisptr, float flFrameTime, float flFrameTimeStdDeviation);
  void (*SetMaxRoutablePayloadSize)(INetChannel *thisptr, int nSplitSize);
  int (*GetMaxRoutablePayloadSize)(INetChannel *thisptr);
  int (*GetProtocolVersion)(INetChannel *thisptr);
};

struct CNetChannel;

struct CNetChannel_vft : INetChannel_vft {
  ConnectionStatus_t (*GetConnectionState)(CNetChannel *thisptr);
};

struct CUDPSocket { };
struct INetworkMessageHandler { };

enum netadrtype_t
{
  NA_NULL = 0x0,
  NA_LOOPBACK = 0x1,
  NA_BROADCAST = 0x2,
  NA_IP = 0x3,
};


struct netadr_t
{
  netadrtype_t type;
  uint8_t ip[4];
  uint16_t port;
  uint8_t _pad0[2];
};

enum e_network_flows
{
  FLOW_OUTGOING = 0x0,
  FLOW_INCOMING = 0x1,
};


// ! member fields could be in different order
struct CNetChannel {
  CNetChannel_vft *vft;
  ConnectionStatus_t m_ConnectionState;
  int m_nOutSequenceNr;
  int m_nInSequenceNr;
  int m_nOutSequenceNrAck;
  int m_nOutReliableState;
  int m_nInReliableState;
  int m_nChokedPackets;
  int m_PacketDrop;
  bf_write m_StreamReliable;
  uint8_t m_ReliableDataBuffer[8192];
  CUtlVector<uint8_t> m_ReliableDataBufferMP;
  bf_write m_StreamUnreliable;
  uint8_t m_UnreliableDataBuffer[1400];
  CUDPSocket *m_pSocket;
  int m_StreamSocket;
  unsigned int m_MaxReliablePayloadSize;
  netadr_t remote_address;
  float last_received;
  float connect_time;
  int m_Rate;
  float m_fClearTime;
  float m_Timeout;
  char m_Name[32];
  INetworkMessageHandler *m_MessageHandler;
};

struct CEngineServer;

struct CEngineServer_vft {
  void *fun_0x00[20];
  INetChannelInfo *(*GetPlayerNetInfo)(CEngineServer *, int32_t index);
};

struct CEngineServer {
  CEngineServer_vft *vft;
};

struct CLagCompensationManager;

struct CLagCompensationManager_vft {
  void (*StartLagCompensation)(CLagCompensationManager *, uintptr_t *plr, CUserCmd *cmd);
  void *fun_0x08[20];
};

struct CLagCompensationManager {
  CLagCompensationManager_vft *vft;
};
