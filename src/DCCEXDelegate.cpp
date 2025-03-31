#include "DCCEXDelegate.h"

DCCEXProtocol DCCEX;

DCCEXDelegateClass::DCCEXDelegateClass() {
}

void DCCEXDelegateClass::init(Stream *stream) {
  DCCEX.setLogStream(&Serial);
  DCCEX.setDelegate(&DCCEXDelegate);
  DCCEX.connect(stream);
  DCCEX.requestServerVersion();
}

void DCCEXDelegateClass::receivedMessage(char *message) {
  DIAG("Received message: %s\n", message);
}

void DCCEXDelegateClass::receivedServerVersion(int major, int minor, int patch) {
  DIAG("Received version: %d.%d.%d\n", major, minor, patch);
}

void DCCEXDelegateClass::receivedTrackPower(TrackPower state) {
  DIAG("Received Track Power: %u\n", state);
  if ( CommandStation.setPower((uint8_t)state) ) {
    LocoNet.reportPower((uint8_t)state);
  }
}
  
void DCCEXDelegateClass::receivedLocoUpdate(Loco *loco) {
  DIAG("Received Loco update for DCC address: %u\n", loco->getAddress());
}
 
void DCCEXDelegateClass::receivedLocoBroadcast(int addr, int speed, Direction direction, int functionMap) {
  DIAG("Received Loco broadcast: address=%u|speed=%u|direction=%u|functionMap=%u\n", addr, speed, (uint8_t)direction, functionMap);

  uint8_t id = CommandStation.addSlot((uint16_t)addr, 0);

  if ( id > 0 && id < 128 ) {

    if ( CommandStation.getSlotStatus(addr) == SL_STATUS_UNSYNCED ) {
      DIAG("unsynced status\n");
      DIAG("func stored %u func received=%u\n",CommandStation.getSlotFunctions(addr), functionMap);
      DIAG("speed stored %u speed received=%u\n",CommandStation.getSlotSpeed(addr), speed);
      if ( CommandStation.getSlotFunctions(addr) == (uint16_t)functionMap && CommandStation.getSlotSpeed(addr) == (uint8_t)speed) { 
        DIAG("set synced status\n");
        CommandStation.setSlotStatus(addr, SL_STATUS_SYNCED); 
      }
    }
    
    if ( CommandStation.getSlotStatus(addr) == SL_STATUS_SYNCED ) {
      DIAG("synced status\n");
      if ( CommandStation.setSlotSpeed(addr, speed) ) { LocoNet.send(OPC_LOCO_SPD, id, speed); }
      if ( CommandStation.setSlotDirection(addr, direction) ) { LocoNet.send(OPC_LOCO_DIRF, id, CommandStation.getSlotDIRF(addr)); }
      if ( CommandStation.setSlotFunctions(addr, functionMap) )
      {
        LocoNet.send(OPC_LOCO_DIRF, id, CommandStation.getSlotDIRF(addr));
        LocoNet.send(OPC_LOCO_SND, id, CommandStation.getSlotSND(addr));
      }
    }
  }
}

void DCCEXDelegateClass::receivedSensorState(int addr, bool state) {
  DIAG("Received Sensor state changed: id=%u state=%u\n", addr, state);
  LocoNet.reportSensor(addr, (uint8_t)state);
}

void DCCEXDelegateClass::sendTrackPower(uint8_t state) { 
  state ? DCCEX.powerOn() : DCCEX.powerOff();
}

void DCCEXDelegateClass::sendEmergencyStop() { DCCEX.emergencyStop(); };

void DCCEXDelegateClass::sendThrottle(uint16_t addr, uint8_t speed, uint8_t dir) {
  Loco loco(addr, LocoSourceEntry);
  char command[18];

  CommandStation.setSlotStatus(addr, SL_STATUS_UNSYNCED);

  sprintf(command, "t %u", addr);
  DCCEX.setThrottle(&loco, speed, (Direction)dir );
  DCCEX.sendCommand(command);
}

void DCCEXDelegateClass::sendFunctions(uint16_t addr, uint16_t functionMap) {
  Loco loco(addr, LocoSourceEntry);
  char command[18];

  CommandStation.setSlotStatus(addr, SL_STATUS_UNSYNCED);

  for (uint8_t i=0; i<9; i++) {
    (functionMap >> i) & 0x1 ? DCCEX.functionOn(&loco, i) : DCCEX.functionOff(&loco, i);
  }

  sprintf(command, "t %u", addr);
  DCCEX.sendCommand(command);
}

void DCCEXDelegateClass::sendSwitchRequest(uint16_t addr, uint8_t out, uint8_t dir) {
  dir ? DCCEX.throwTurnout(addr): DCCEX.closeTurnout(addr);
}

void DCCEXDelegateClass::check() {
  DCCEX.check();
}
DCCEXDelegateClass DCCEXDelegate = DCCEXDelegateClass();