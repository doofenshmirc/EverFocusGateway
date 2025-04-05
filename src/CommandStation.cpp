#include "CommandStation.h"

CommandStationClass::CommandStationClass() {
}

void CommandStationClass::init() {
}

uint16_t CommandStationClass::getSlotAddress(uint8_t id) {
  SlotClass *slot = _getSlotById(id);
  
  if ( slot ) { return slot->getAddress(); }

  return 0;
}

uint8_t CommandStationClass::addSlot(uint16_t addr, uint8_t id) {
  SlotClass *slot = _getSlotByAddress(addr);

  if (!slot) { 
    slot = new SlotClass(addr, id); 
  }

  if (slot) { 
    if ( slot->getId() == 0 && id == 0 ) LocoNetInterface.send(OPC_LOCO_ADR, (addr & 0xFF80) >> 7, addr & 0x7F ); 
    else if ( slot->getId() != id && id != 0 ) slot->setId(id); 

    return slot->getId();
  }

  return 0x80;
}

uint8_t CommandStationClass::getSlotStatus(uint16_t addr) {
  SlotClass *slot = _getSlotByAddress(addr);
  
  if (slot) { return slot->getSlotStatus(); }
  
  return 128;
}

void CommandStationClass::setSlotStatus(uint16_t addr, uint8_t status) {
  SlotClass *slot = _getSlotByAddress(addr);
  
  if (slot) { slot->setSlotStatus(status); }
}

uint8_t CommandStationClass::getSlotSpeed(uint16_t addr) {
  SlotClass *slot = _getSlotByAddress(addr);
  
  if ( slot ) { return slot->getSpeed(); }

  return 128;
}

void CommandStationClass::setSlotSpeed(uint16_t addr, uint8_t speed, uint8_t src) {
  SlotClass *slot = _getSlotByAddress(addr);
  
  if ( slot ) { slot->setSpeed(speed, src); }
}

uint8_t CommandStationClass::getSlotDirection(uint16_t addr) {
  SlotClass *slot = _getSlotByAddress(addr);
  
  if ( slot ) { return slot->getDirection(); }

  return Direction::Forward;
}

void CommandStationClass::setSlotDirection(uint16_t addr, uint8_t dir, uint8_t src) {
  SlotClass *slot = _getSlotByAddress(addr);
  
  if ( slot ) { slot->setDirection(dir, src); }
}

uint16_t CommandStationClass::getSlotFunctions(uint16_t addr) {
  SlotClass *slot = _getSlotByAddress(addr);
  
  if ( slot ) { return slot->getFunctions(); }

  return 0;
}

void CommandStationClass::setSlotFunctions(uint16_t addr, uint16_t functions, uint8_t src) {
  SlotClass *slot = _getSlotByAddress(addr);

  if ( slot ) { slot->setFunctions(functions, src); }
}

void CommandStationClass::setSensor(uint16_t addr, uint8_t state, uint8_t src) {
  if ( src != SRC_LOCONET ) LocoNetInterface.sendSensor(addr, state);
}

void CommandStationClass::setSwitch(uint16_t addr, uint8_t out, uint8_t dir, uint8_t src) {
  if ( src != SRC_DCCEX ) DCCEXInterface.sendSwitch(addr, out, dir);      
  if ( src != SRC_LOCONET ) LocoNetInterface.sendSwitch(addr, out, dir);
}

void CommandStationClass::setPower(uint8_t power, uint8_t src) {
  if (_power != power) {
    _power = power;
    if ( src != SRC_LOCONET ) LocoNetInterface.sendPower(power);
    if ( src != SRC_DCCEX ) DCCEXInterface.sendPower(power);
  }
}

void CommandStationClass::EmergencyStop(uint8_t src) {
  if ( src != SRC_DCCEX ) DCCEXInterface.sendEmergencyStop();
  if ( src != SRC_LOCONET ) LocoNetInterface.sendEmergencyStop();
}

void CommandStationClass::check() {
  for (SlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if ( s->getSourceStatus() != SRC_NULL ) {
      if ( s->getSourceStatus() != SRC_LOCONET ) {
        if ( s->getSpeedStatus() ) LocoNetInterface.sendSpeed(s->getId(), s->getSpeed());
        if ( s->getDirectionStatus() || (s->getFunctionsStatus() & 0x1F)) LocoNetInterface.sendDirFunct0to4(s->getId(), s->getDirection(), s->getFunctions());
        if ( s->getFunctionsStatus() & 0x1E0) LocoNetInterface.sendFunct5to8(s->getId(), s->getFunctions());
      }
    
      if ( s->getSourceStatus() != SRC_DCCEX ) {
        if ( s->getSpeedStatus() || s->getDirectionStatus() ) DCCEXInterface.sendThrottle(s->getAddress(), s->getSpeed(), s->getDirection());
        if ( s->getFunctionsStatus() ) DCCEXInterface.sendFunctions(s->getAddress(), s->getFunctions(), s->getFunctionsStatus());
      }

      //s->setSlotStatus(SLOT_STAT_READY);
      s->setSpeedStatus(SRC_NULL);
      s->setDirectionStatus(SRC_NULL);
      s->setFunctionsStatus(SRC_NULL);
    }
  }
}

// private methods
SlotClass *CommandStationClass::_getSlotByAddress(uint16_t addr) {
  for (SlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getAddress() == addr) {
      return s;
    }
  }
  return nullptr;
}
SlotClass *CommandStationClass::_getSlotById(uint8_t id) {
  for (SlotClass *s = _slots->getFirst(); s; s = s->getNext()) {
    if (s->getId() == id) {
      return s;
    }
  }
  return nullptr;
}

CommandStationClass CommandStation = CommandStationClass();