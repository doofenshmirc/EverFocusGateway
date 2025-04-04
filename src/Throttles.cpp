#include "Throttles.h"
#include <Arduino.h>

ThrottleClass *ThrottleClass::_first = nullptr;

ThrottleClass::ThrottleClass(uint16_t addr) {
  _addr = addr;
  _name = nullptr;
  _next = nullptr;
 
  if (!_first) {
    _first = this;
  } else {
    ThrottleClass *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = this;
  }
}

void ThrottleClass::setThrottle(uint8_t speed, uint8_t dir) { 
  uint8_t id = CommandStation.addSlot(_addr, 0);

  if ( CommandStation.setSlotThrottle(_addr, speed, dir) ) {
    DCCEXDelegate.sendThrottle(_addr, speed, dir);
  }
}

void ThrottleClass::setSpeed(uint8_t speed) { 
  _speed = speed;
  if (_dccexProtocol) {
    _loco->setSpeed(speed);
    _dccexProtocol->setThrottle(_loco, _loco->getSpeed(), _loco->getDirection());
  }    
//  if (_loconetProtocol) {
//    _loconetProtocol->send(OPC_LOCO_SPD, _slot, speed);
//  }
}

uint8_t ThrottleClass::getSpeed() { return _speed; }

void ThrottleClass::setDirection(uint8_t direction) {
  _direction = direction;
  if (_dccexProtocol) {
    _loco->setDirection(direction ? Direction::Forward : Direction::Reverse); 
    _dccexProtocol->setThrottle(_loco, _loco->getSpeed(), _loco->getDirection());
  }
}

uint8_t ThrottleClass::getDirection() { return _direction; }

void ThrottleClass::setName(const char *name) {
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

void ThrottleClass::setFunction(uint8_t function, uint8_t value) {
  value ? _functions |= (1 << function) : _functions &= ~(1 << function);

  if (_dccexProtocol) {
    _loco->setFunctionStates(_functions); 
    value ? _dccexProtocol->functionOn(_loco, function) : _dccexProtocol->functionOff(_loco, function);
  }
}
const char *ThrottleClass::getName() { return _name; }

void ThrottleClass::setSlot(uint8_t slot ) { _slot = slot; }

uint8_t ThrottleClass::getSlot() { return _slot; }

void ThrottleClass::setAddress(uint16_t address ) { _address = address; }

uint16_t ThrottleClass::getAddress() { return _address; }

ThrottleClass *ThrottleClass::getFirst() { return _first; }

void ThrottleClass::setNext(ThrottleClass *throttle) { _next = throttle; }

ThrottleClass *ThrottleClass::getNext() { return _next; }

ThrottleClass *ThrottleClass::getByAddress(uint16_t address) {
  for (ThrottleClass *t = ThrottleClass::getFirst(); t; t = t->getNext()) {
    if (t->getAddress() == address) {
      return t;
    }
  }
  return nullptr;
}

void ThrottleClass::clearThrottleList() {
  // Count Throttle in list
  int throttleCount = 0;
  ThrottleClass *currentThrottle = ThrottleClass::getFirst();
  while (currentThrottle != nullptr) {
    throttleCount++;
    currentThrottle = currentThrottle->getNext();
  }

  // Store Turnout pointers in an array for clean up
  ThrottleClass **deleteThrottle = new ThrottleClass *[throttleCount];
  currentThrottle = ThrottleClass::getFirst();
  for (int i = 0; i < throttleCount; i++) {
    deleteThrottle[i] = currentThrottle;
    currentThrottle = currentThrottle->getNext();
  }

  // Delete each Turnout
  for (int i = 0; i < throttleCount; i++) {
    delete deleteThrottle[i];
  }

  // Clean up the array of pointers
  delete[] deleteThrottle;

  // Reset first pointer
  ThrottleClass::_first = nullptr;
}

ThrottleClass::~Throttle() {
  _removeFromList(this);

  if (_name) {
    delete[] _name;
    _name = nullptr;
  }

  _next = nullptr;
}

// Private methods

void ThrottleClass::_removeFromList(ThrottleClass *throttle) {
  if (!throttle) {
    return;
  }

  if (getFirst() == throttle) {
    _first = throttle->getNext();
  } else {
    ThrottleClass *currentThrottle = _first;
    while (currentThrottle && currentThrottle->getNext() != throttle) {
      currentThrottle = currentThrottle->getNext();
    }
    if (currentThrottle) {
      currentThrottle->setNext(throttle->getNext());
    }
  }
}

extern ThrottleClass *Throttles = nullptr;