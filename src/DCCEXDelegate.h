#pragma once

#include <DCCEXProtocol.h>
#include "CommandStation.h"
#include "diag.h"

class DCCEXDelegateClass : public DCCEXProtocolDelegate {
  public:
    DCCEXDelegateClass();
    void init(Stream *stream);
    void receivedMessage(char *message);
    void receivedServerVersion(int major, int minor, int patch) override;
    void receivedTrackPower(TrackPower state) override;
    void receivedLocoUpdate(Loco *loco) override;
    void receivedLocoBroadcast(int addr, int speed, Direction direction, int functionMap) override;
    void receivedSensorState(int addr, bool state);
    void sendTrackPower(uint8_t state);
    void sendEmergencyStop();
    void sendThrottle(uint16_t addr, uint8_t speed, uint8_t dir);
    void sendFunctions(uint16_t addr, uint16_t functionMap);
    void sendSwitchRequest(uint16_t addr, uint8_t out, uint8_t dir);
    void check();
};

extern DCCEXDelegateClass DCCEXDelegate;
extern DCCEXProtocol DCCEX;