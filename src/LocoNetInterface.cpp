#include "LocoNetInterface.h"

LocoNetInterfaceClass::LocoNetInterfaceClass() {
}
  
void LocoNetInterfaceClass::init(uint8_t txPin) {
  LocoNet.init(txPin);
}

LN_STATUS LocoNetInterfaceClass::send(uint8_t OpCode, uint8_t Data1, uint8_t Data2) {
  return LocoNet.send(OpCode, Data1, Data2);
}

void LocoNetInterfaceClass::reportSwitch(uint16_t addr) {
  LocoNet.reportSwitch(addr);
}

void LocoNetInterfaceClass::sendSpeed(uint8_t id, uint8_t speed) {
  LocoNet.send(OPC_LOCO_SPD, id, speed);
}

void LocoNetInterfaceClass::sendDirFunct0to4(uint8_t id, uint8_t direction, uint16_t functions) {
  uint8_t dirFunct0to4 = (direction << 5) | ((functions & 0x1) << 4) | ((functions & 0x1E) >> 1);
  DIAG("LocoNetInterfaceClass::sendDirFunct0to4 id:%u, dir:%u, func:%u\n", id, direction, functions);
  LocoNet.send(OPC_LOCO_DIRF, id, dirFunct0to4);
}

void LocoNetInterfaceClass::sendFunct5to8(uint8_t id, uint16_t functions) {
  uint8_t funct5to8 = ((functions & 0x1E0) >> 5);
  LocoNet.send(OPC_LOCO_SND, id, funct5to8);
}

void LocoNetInterfaceClass::sendSwitch(uint16_t addr, uint8_t out, uint8_t dir) {
  LocoNet.requestSwitch(addr, out, dir);
}

void LocoNetInterfaceClass::sendSensor(uint16_t addr, uint8_t state) {
  LocoNet.reportSensor(addr, state);
}
  
void LocoNetInterfaceClass::sendPower(uint8_t power) {
  LocoNet.reportPower(power);
}

void LocoNetInterfaceClass::sendEmergencyStop() {
  LocoNet.send(OPC_IDLE, 0, 0);
}

void LocoNetInterfaceClass::processMessage(lnMsg *lnPacket) {
  uint16_t addr;
  uint8_t id;
  
  DIAG("Free RAM: %d\n", freeMemory());
  uint8_t msgLen = getLnMsgSize(lnPacket);
  DIAG("Receive loconet packet: ");
  for (uint8_t x = 0; x < msgLen; x++)
  {
    DIAG("%02X ", lnPacket->data[x]);
  }
  DIAG("\n");
  
  switch(lnPacket->sd.command) {
    case OPC_GPON:
      CommandStation.setPower(POWER_ON, SRC_LOCONET);
      break;

    case OPC_GPOFF:
      CommandStation.setPower(POWER_OFF, SRC_LOCONET);
      break;

    case OPC_IDLE:
      CommandStation.EmergencyStop(SRC_LOCONET);
      break;

    case OPC_WR_SL_DATA:
    case OPC_SL_RD_DATA:
      addr = ((uint16_t)lnPacket->sd.adr2 << 7) + lnPacket->sd.adr;
      id = CommandStation.addSlot(addr, lnPacket->sd.slot);
      DIAG("LocoNetInterfaceClass::processMessage OPC_SL_RD_DATA addr:%u, id:%u\n", addr, id);

      if ( id > 0 && id < 128 ) {
        CommandStation.setSlotSpeed(addr, lnPacket->sd.spd, SRC_LOCONET);
        CommandStation.setSlotDirection(addr, (Direction)((lnPacket->sd.dirf & 0x20) >> 5), SRC_LOCONET);
        CommandStation.setSlotFunctions(addr, (CommandStation.getSlotFunctions(addr) & 0xFE00) | (((uint16_t)lnPacket->sd.snd & 0x0F) << 5 ) | (((uint16_t)lnPacket->sd.dirf & 0xF) << 1) | (((uint16_t)lnPacket->sd.dirf & 0x10) >> 4), SRC_LOCONET);
        
      }
      break;

    case OPC_LOCO_SPD:
      addr = CommandStation.getSlotAddress(lnPacket->ld.slot);
      DIAG("LocoNetInterfaceClass::processMessage OPC_LOCO_SPD addr:%u, speed:%u\n", addr, lnPacket->ld.data);

      if ( addr > 0 ) {  
        CommandStation.setSlotSpeed(addr, lnPacket->ld.data, SRC_LOCONET);
      } else LocoNet.send(OPC_RQ_SL_DATA, lnPacket->ld.slot, 0);
      break;

    case OPC_LOCO_DIRF:
      addr = CommandStation.getSlotAddress(lnPacket->ld.slot);
      DIAG("LocoNetInterfaceClass::processMessage OPC_LOCO_DIRF addr:%u, dirf:%u\n", addr, lnPacket->ld.data);

      if ( addr > 0 ) { 
        CommandStation.setSlotDirection(addr, ((lnPacket->ld.data & 0x20) >> 5), SRC_LOCONET);
        CommandStation.setSlotFunctions(addr, (CommandStation.getSlotFunctions(addr) & 0xFFE0) | (((uint16_t)lnPacket->ld.data & 0xF) << 1) | (((uint16_t)lnPacket->ld.data & 0x10) >> 4), SRC_LOCONET);
      } else LocoNet.send(OPC_RQ_SL_DATA, lnPacket->ld.slot, 0);
      break;

    case OPC_LOCO_SND:
      addr = CommandStation.getSlotAddress(lnPacket->ld.slot);
      DIAG("LocoNetInterfaceClass::processMessage OPC_LOCO_SND addr:%u, snd:%u\n", addr, lnPacket->ld.data);

      if ( addr > 0 ) {
        if ( CommandStation.setSlotFunctions(addr, (CommandStation.getSlotFunctions(addr) &0xFE1F ) | ((uint16_t)lnPacket->ld.data << 5), SRC_LOCONET), SRC_LOCONET);
      } else LocoNet.send(OPC_RQ_SL_DATA, lnPacket->ld.slot, 0);
      break;

    case OPC_SLOT_STAT1:
      addr = CommandStation.getSlotAddress(lnPacket->ld.slot);
      DIAG("LocoNetInterfaceClass::processMessage OPC_SLOT_STAT1 addr:%u, snd:%u\n", addr, lnPacket->ld.data);

      if ( addr > 0 ) {
        CommandStation.setSlotStatus(addr, lnPacket->ld.data);
      } else LocoNet.send(OPC_RQ_SL_DATA, lnPacket->ld.slot, 0);
      break;
    
    case OPC_SW_REQ:
      addr = (lnPacket->srq.sw1 | ((lnPacket->srq.sw2 & 0x0F) << 7)) + 1;
      DIAG("LocoNetInterfaceClass::processMessage OPC_SW_REQ addr:%u, out:%u, dir:%u\n", addr, lnPacket->srq.sw2 & OPC_SW_REQ_OUT, lnPacket->srq.sw2 & OPC_SW_REQ_DIR);

      CommandStation.setSwitch(addr, lnPacket->srq.sw2 & OPC_SW_REQ_OUT, lnPacket->srq.sw2 & OPC_SW_REQ_DIR, SRC_LOCONET);
      break;  

    default:
        DIAG("# !! Ignore LocoNet message !! #\n");      
        break;
  }
}

void LocoNetInterfaceClass::check() {
  lnMsg *lnPacket = LocoNet.receive();

  if ( lnPacket ) {
    processMessage(lnPacket);
  }
}

LocoNetInterfaceClass LocoNetInterface = LocoNetInterfaceClass();