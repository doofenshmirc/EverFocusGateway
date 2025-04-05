#pragma once

#include <LocoNet.h>
#include "CommandStation.h"
#include "config.h"

class LocoNetInterfaceClass {
  public:
    LocoNetInterfaceClass();
    void init(uint8_t txPin);
    LN_STATUS send(uint8_t OpCode, uint8_t Data1, uint8_t Data2);
    void reportSwitch(uint16_t addr);
    void sendSpeed(uint8_t id, uint8_t speed);
    void sendDirFunct0to4(uint8_t id, uint8_t direction, uint16_t functions);
    void sendFunct5to8(uint8_t id, uint16_t functions);
    void sendSwitch(uint16_t addr, uint8_t out, uint8_t dir);
    void sendSensor(uint16_t addr, uint8_t state);
    void sendPower(uint8_t power);
    void sendEmergencyStop();
    void processMessage(lnMsg *lnPacket);
    void check();
  private:
    LocoNetClass *_loconet = &LocoNet;
};

extern LocoNetInterfaceClass LocoNetInterface;
