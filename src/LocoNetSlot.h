#ifndef LocoNetSlot_H
#define LocoNetSlot_H

#include <Arduino.h>

#define SL_STATUS_SYNCED   0x00
#define SL_STATUS_UNSYNCED 0x01

class LocoNetSlotClass {
public:
  LocoNetSlotClass(uint16_t addr, uint8_t id);

  uint8_t getId() { return _id; };

  bool setId(uint8_t id);

  uint16_t getAddress() { return _addr; };
  
  bool setAddress(uint16_t addr);

  uint8_t getStatus() { return _status; };
  
  void setStatus(uint8_t status) {_status = status; };

  uint8_t getSpeed() { return _speed; };
  
  bool setSpeed(uint8_t speed);

  uint8_t getDirection() { return _dir; };
  
  bool setDirection(uint8_t dir);

  uint16_t getFunctions() { return _functionMap; };

  bool setFunctions(uint16_t functionMap);

  static LocoNetSlotClass *getFirst();

  void setNext(LocoNetSlotClass *slot);

  LocoNetSlotClass *getNext();

  /// @brief Clear the list of Slot
  static void clearSlotList();

  /// @brief Destructor for a Slot
  ~LocoNetSlotClass();

private:
  uint8_t _id;
  uint16_t _addr;
  uint8_t _status;
  uint8_t _speed;
  uint8_t _dir;
  uint16_t _functionMap;
  static LocoNetSlotClass *_first;
  LocoNetSlotClass *_next;

  void _removeFromList(LocoNetSlotClass *slot);
};

extern LocoNetSlotClass *Slots;

#endif