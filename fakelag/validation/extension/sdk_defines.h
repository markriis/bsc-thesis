#pragma once

typedef unsigned char byte;

typedef float float32_t;
typedef double float64_t;

class QAngle {
public:
    float x, y, z;
};

struct CUserCmd {
  void *vft;
  int32_t command_number;
  uint8_t pad_0x00[4];
  QAngle viewangles__0x10;
  float32_t forwardmove;
  float32_t sidemove;
  float32_t upmove;
  int32_t dropped_packets__0x28;
  int8_t impulse__0x2C;
  uint8_t pad_0x2C[11];
  int random_seed__0x38;
  int server_random_seed__0x3C;
  uint8_t pad_0x40[8];
};

struct CGlobalVars {
  uint8_t pax_0x0[24];
  int32_t tickcount;
  float interval_per_tick__0x1C;
};
