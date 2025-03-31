#include "LocoNetSlot.h"

LocoNetSlotClass *LocoNetSlotClass::_first = nullptr;

LocoNetSlotClass::LocoNetSlotClass(uint16_t addr, uint8_t id) {
  _id = id;
  _addr = addr;
  _status = SL_STATUS_UNSYNCED;
  _functionMap = 0;
  _next = nullptr;

  if (!_first) {
    _first = this;
  } else {
    LocoNetSlotClass *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = this;
  }
}

bool LocoNetSlotClass::setId(uint8_t id) {
  if ( _id != id) {
    _id = id;
    return true;
  }
  return false;
}

bool LocoNetSlotClass::setAddress(uint16_t addr) {
  if ( _addr != addr) {
    _addr = addr;
    return true;
  }
  return false;
}

bool LocoNetSlotClass::setSpeed(uint8_t speed) {
  if ( _speed != speed) {
    _speed = speed;
    return true;
  }
  return false;
}

bool LocoNetSlotClass::setDirection(uint8_t dir) {
  if ( _dir != dir) {
    _dir = dir;
    return true;
  }
  return false;
}

bool LocoNetSlotClass::setFunctions(uint16_t functionMap) {
  if ( _functionMap != functionMap) {
    _functionMap = functionMap;
    return true;
  }
  return false;
}

LocoNetSlotClass *LocoNetSlotClass::getFirst() { return _first; }

void LocoNetSlotClass::setNext(LocoNetSlotClass *next) { _next = next; }

LocoNetSlotClass *LocoNetSlotClass::getNext() { return _next; }

LocoNetSlotClass::~LocoNetSlotClass() {
  _removeFromList(this);

  _next = nullptr;
}

// Private methods

void LocoNetSlotClass::_removeFromList(LocoNetSlotClass *slot) {
  if (!slot) {
    return;
  }

  if (getFirst() == slot) {
    _first = slot->getNext();
  } else {
    LocoNetSlotClass *currentSlot = _first;
    while (currentSlot && currentSlot->getNext() != slot) {
      currentSlot = currentSlot->getNext();
    }
    if (currentSlot) {
      currentSlot->setNext(slot->getNext());
    }
  }
}

LocoNetSlotClass *Slots = new LocoNetSlotClass(0, 0);