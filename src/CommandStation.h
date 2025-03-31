#pragma once

#include <Arduino.h>
#include <LocoNet.h>
#include "LocoNetSlot.h"
#include "DCCEXDelegate.h"
#include "diag.h"

class CommandStationClass {
    public:
      CommandStationClass();

      void init(uint8_t txPin);

      bool setPower(uint8_t power);

      uint8_t getPower();

      uint8_t addSlot(uint16_t addr, uint8_t id);

      uint16_t getSlotAddress(uint8_t id);

      void setSlotStatus(uint16_t addr, uint8_t status);

      uint8_t getSlotStatus(uint16_t addr);

      bool setSlotSpeed(uint16_t addr, uint8_t speed);

      uint8_t getSlotSpeed(uint16_t addr);

      bool setSlotDirection(uint16_t addr, uint8_t dir);
      
      uint8_t getSlotDirection(uint16_t addr);

      bool setSlotThrottle(uint16_t addr, uint8_t speed, uint8_t dir);
      
      bool setSlotFunctions(uint16_t addr, uint16_t functionMap);

      uint16_t getSlotFunctions(uint16_t addr);

      uint8_t getSlotDIRF(uint16_t addr);

      uint8_t getSlotSND(uint16_t addr);

      void powerOn();

      void powerOff();
      
      void emergencyStop();

      void processLoconetPacket(lnMsg *lnPacket);

      void check();

    private:
      uint8_t _power = 2;
      LocoNetSlotClass *_slots = nullptr;
};

extern CommandStationClass CommandStation;