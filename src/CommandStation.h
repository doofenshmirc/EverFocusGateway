#pragma once

#include "DCCEXInterface.h"
#include "LocoNetInterface.h"
#include "Slot.h"
#include "MemoryFree.h"
#include "config.h"
#include "diag.h"

#define CS_POWER_CHANGE  0x01
#define POWER_OFF 0
#define POWER_ON  1

class CommandStationClass {
  public:
    CommandStationClass();

    void init();

    uint16_t getSlotAddress(uint8_t id);

    uint8_t addSlot(uint16_t addr, uint8_t id);

    uint8_t getSlotStatus(uint16_t addr);
    void setSlotStatus(uint16_t addr, uint8_t status);
    
    uint8_t getSlotSpeed(uint16_t addr);
    void setSlotSpeed(uint16_t addr, uint8_t speed, uint8_t src);

    uint8_t getSlotDirection(uint16_t addr);
    void setSlotDirection(uint16_t addr, uint8_t dir, uint8_t src);

    uint16_t getSlotFunctions(uint16_t addr);
    void setSlotFunctions(uint16_t addr, uint16_t functions, uint8_t src);

    void setSensor(uint16_t addr, uint8_t state, uint8_t src);

    void setSwitch(uint16_t addr, uint8_t out, uint8_t dir, uint8_t src);

    uint8_t getPower() { return _power; };
    void setPower(uint8_t power, uint8_t src);

    void EmergencyStop(uint8_t src);

    void check();

  private:
    uint8_t _power;
    uint8_t _status;
    SlotClass *_slots = nullptr;
    SlotClass *_getSlotById(uint8_t id);
    SlotClass *_getSlotByAddress(uint16_t addr);
};

extern CommandStationClass CommandStation;