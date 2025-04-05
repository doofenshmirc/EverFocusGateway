#pragma once

#include <DCCEXProtocol.h>
#include "CommandStation.h"
#include "diag.h"

class DCCEXInterfaceClass : public DCCEXProtocolDelegate {
  public:
    DCCEXInterfaceClass();
    void init(Stream *client, Stream *log);
    void receivedMessage(char *message);
    void receivedServerVersion(int major, int minor, int patch) override;
    void receivedTrackPower(TrackPower state) override;
    void receivedLocoUpdate(Loco *loco) override;
    void receivedLocoBroadcast(int addr, int speed, Direction direction, int functionMap) override;
    void receivedSensorState(int addr, bool state);
    void sendThrottle(uint16_t addr, uint8_t speed, uint8_t dir);
    void sendFunctions(uint16_t addr, uint16_t functions, uint16_t funct_status);
    void sendSwitch(uint16_t addr, uint8_t out, uint8_t dir);
    void sendPower(uint8_t power);
    void sendEmergencyStop();
    void check();
  private:
    DCCEXProtocol _dccex;    
};

extern DCCEXInterfaceClass DCCEXInterface;
