#include "CommandStation.h"
#include <MemoryFree.h>

CommandStationClass::CommandStationClass() {
  _slots = new LocoNetSlotClass(0, 0);
}

void CommandStationClass::init(uint8_t txPin) {
  LocoNet.init(txPin);
}

bool CommandStationClass::setPower(uint8_t power) {
  if ( _power != power) {
    _power = power;
    return true;
  }
  return false;
}

uint8_t CommandStationClass::getPower() {
  return _power;
}

uint8_t CommandStationClass::addSlot(uint16_t addr, uint8_t id) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      if (id != 0 ) s->setId(id);
      return s->getId();
    }
  }

  new LocoNetSlotClass(addr, id);

  if ( id == 0 ) { LocoNet.send(OPC_LOCO_ADR, (addr & 0xFF80) >> 7, addr & 0x7F ); }

  return id;
}

uint16_t CommandStationClass::getSlotAddress(uint8_t id) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getId() == id) {
      return s->getAddress();
    }
  }
  return 0;
}

void CommandStationClass::setSlotStatus(uint16_t addr, uint8_t status) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      s->setStatus(status);
    }
  }
}

uint8_t CommandStationClass::getSlotStatus(uint16_t addr) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s->getStatus();
    }
  }
  return SL_STATUS_UNSYNCED;
}

bool CommandStationClass::setSlotSpeed(uint16_t addr, uint8_t speed) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s->setSpeed(speed);
    }
  }
  return false;
}

uint8_t CommandStationClass::getSlotSpeed(uint16_t addr) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s->getSpeed();
    }
  }
  return 0;
}

bool CommandStationClass::setSlotDirection(uint16_t addr, uint8_t dir) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s->setDirection(dir);
    }
  }
  return false;
}

uint8_t CommandStationClass::getSlotDirection(uint16_t addr) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s->getDirection();
    }
  }
  return 0;
}

bool CommandStationClass::setSlotThrottle(uint16_t addr, uint8_t speed, uint8_t dir) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      bool isSpeedChange = s->setSpeed(speed);
      bool isDirChange = s->setDirection(dir);
      return ( isSpeedChange || isDirChange );
    }
  }
  return false;
}

bool CommandStationClass::setSlotFunctions(uint16_t addr, uint16_t functionMap) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s->setFunctions(functionMap);
    }
  }
  return false;
}

uint16_t CommandStationClass::getSlotFunctions(uint16_t addr) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s->getFunctions();
    }
  }
  return 0;
}
  
uint8_t CommandStationClass::getSlotDIRF(uint16_t addr) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return (s->getDirection()<<5) | ((s->getFunctions() & 0x1) << 4) | ((s->getFunctions() & 0x1E) >> 1);
    }
  }
  return 0;
}

uint8_t CommandStationClass::getSlotSND(uint16_t addr) {
  for (LocoNetSlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return ((s->getFunctions() & 0x1E0) >> 5);
    }
  }
  return 0;
}

void CommandStationClass::processLoconetPacket(lnMsg *lnPacket) {
  uint8_t id;
  uint16_t addr;

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
      if ( setPower(1) ) {
        DCCEXDelegate.sendTrackPower(1);
      }

      break;
    case OPC_GPOFF:
      if ( setPower(0) ) {
        DCCEXDelegate.sendTrackPower(0);
      }

      break;
    case OPC_IDLE:
      DCCEXDelegate.sendEmergencyStop();

      break;
    case OPC_WR_SL_DATA:
    case OPC_SL_RD_DATA:
      addr = ((uint16_t)lnPacket->sd.adr2 << 7) + lnPacket->sd.adr;
      id = addSlot(addr, lnPacket->sd.slot);
      DIAG("CommandStationClass::check OPC_SL_RD_DATA addr:%u, id:%u\n", addr, id);

      if ( id > 0 && id < 128 ) {
        setSlotThrottle(addr, lnPacket->sd.spd, (lnPacket->sd.dirf & 0x20) >> 5);
        DCCEXDelegate.sendThrottle(addr, getSlotSpeed(addr), getSlotDirection(addr));

        setSlotFunctions(addr, (getSlotFunctions(addr) &0xFFFFF600 ) | ((int)lnPacket->sd.snd << 5) | ((lnPacket->sd.dirf & 0xF) << 1) | ((lnPacket->sd.dirf & 0x10) >> 4));  
        DCCEXDelegate.sendFunctions(addr, getSlotFunctions(addr));
      }

      break;
    case OPC_LOCO_SPD:
      addr = getSlotAddress(lnPacket->ld.slot);
      DIAG("CommandStationClass::check OPC_LOCO_SPD addr:%u, speed:%u\n", addr, lnPacket->ld.data);

      if ( addr > 0 ) { 
        if ( setSlotSpeed(addr, lnPacket->ld.data) ) {
          DCCEXDelegate.sendThrottle(addr, getSlotSpeed(addr), getSlotDirection(addr));
        }
      } else LocoNet.send(OPC_RQ_SL_DATA, lnPacket->ld.slot, 0);

      break;
    case OPC_LOCO_DIRF:
      addr = getSlotAddress(lnPacket->ld.slot);
      DIAG("CommandStationClass::check OPC_LOCO_DIRF addr:%u, dirf:%u\n", addr, lnPacket->ld.data);

      if ( addr > 0 ) { 
        if ( setSlotDirection(addr, (lnPacket->ld.data & 0x20) >> 5) ) {
          DCCEXDelegate.sendThrottle(addr, getSlotSpeed(addr), getSlotDirection(addr));
        }
        if ( setSlotFunctions(addr, (getSlotFunctions(addr) &0xFFFFFFE0 ) | ((lnPacket->ld.data & 0xF) << 1) | ((lnPacket->ld.data & 0x10) >> 4)) ) {  
          DCCEXDelegate.sendFunctions(addr, getSlotFunctions(addr));
        }
      } else LocoNet.send(OPC_RQ_SL_DATA, lnPacket->ld.slot, 0);

      break;
    case OPC_LOCO_SND:
      addr = getSlotAddress(lnPacket->ld.slot);
      DIAG("CommandStationClass::check OPC_LOCO_SND addr:%u, snd:%u\n", addr, lnPacket->ld.data);

      if ( addr > 0 ) {
        if ( setSlotFunctions(addr, (getSlotFunctions(addr) &0xFFFFFE1F ) | ((int)lnPacket->ld.data << 5)) ) {
          DCCEXDelegate.sendFunctions(addr, getSlotFunctions(addr));
        }
      } else LocoNet.send(OPC_RQ_SL_DATA, lnPacket->ld.slot, 0);

      break;
    case OPC_SW_REQ:
      addr = (lnPacket->srq.sw1 | ((lnPacket->srq.sw2 & 0x0F) << 7)) + 1;
      DIAG("CommandStationClass::check OPC_SW_REQ addr:%u, out:%u, dir:%u\n", addr, lnPacket->srq.sw2 & OPC_SW_REQ_OUT, lnPacket->srq.sw2 & OPC_SW_REQ_DIR);

      DCCEXDelegate.sendSwitchRequest(addr, lnPacket->srq.sw2 & OPC_SW_REQ_OUT, lnPacket->srq.sw2 & OPC_SW_REQ_DIR);

      break;  
    default:
      DIAG("# !! Ignore LocoNet message !! #\n");      
      break;
  }
}

void CommandStationClass::check() {
  lnMsg *lnPacket = LocoNet.receive();

  if ( lnPacket ) {
    processLoconetPacket(lnPacket);
    LocoNet.processSwitchSensorMessage(lnPacket);
  }
}

CommandStationClass CommandStation = CommandStationClass();
  