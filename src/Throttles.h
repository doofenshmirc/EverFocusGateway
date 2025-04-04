#ifndef THROTTLES_H
#define THROTTLES_H

#include <LocoNet.h>
#include <DCCEXProtocol.h>
#include "CommandStation.h"
//#include <XpressNet.h>

/// @brief Class to contain and maintain the various Turnout/Point attributes and methods
class ThrottleClass {
public:
  ThrottleClass(uint16_t addr);

  void process() {
    // Routine calls here included in the loop to read encoder or other inputs
  }

  void setThrottle(uint8_t speed, uint8_t dir);

  /// @brief Set thrown state (true thrown, false closed)
  /// @param speed 0-127
  void setSpeed(uint8_t speed);

  /// @brief Get loco speed
  /// @param direction true|false
  uint8_t getSpeed();

  /// @brief Set loco direction
  /// @param direction true|false
  void setDirection(uint8_t dir);

  /// @brief Get loco direction
  /// @param direction true|false
  uint8_t getDirection();

  /// @brief Set loco direction
  /// @param direction true|false
  void setFunction(uint8_t function, uint8_t value);

  /// @brief Set turnout name
  /// @param _name Name to set the turnout
  void setName(const char *_name);

  /// @brief Get turnout name
  /// @return Current name of the turnout
  const char *getName();

  /// @brief Set LocoNet slot ID
  /// @return ID of the Loconet slot
  void setSlot(uint8_t slot);

  /// @brief Get turnout Id
  /// @return ID of the turnout
  uint8_t getSlot();

  /// @brief Get thrown state (true thrown, false closed)
  /// @return true|false
  void setAddress(uint16_t addr);

  /// @brief Get thrown state (true thrown, false closed)
  /// @return true|false
  uint16_t getAddress();

  /// @brief Get first turnout object
  /// @return Pointer to the first Turnout object
  static ThrottleClass *getFirst();

  /// @brief Set the next turnout in the list
  /// @param turnout Pointer to the next Turnout
  void setNext(ThrottleClass *throttle);

  /// @brief Get next turnout object
  /// @return Pointer to the next Turnout object
  ThrottleClass *getNext();

  /// @brief Get turnout object by turnout ID
  /// @param id ID of the turnout to retrieve
  /// @return Pointer to the turnout object or nullptr if not found
  static ThrottleClass *getByAddress(uint16_t address);

  /// @brief Clear the list of turnouts
  static void clearThrottleList();

  /// @brief Destructor for a Turnout
  ~ThrottleClass();

private:
  uint16_t _addr;
  static ThrottleClass *_first;
  ThrottleClass *_next;
  char *_name;

  /// @brief Remove the throttle from the list
  /// @param throttle Pointer to the throttle to remove
  void _removeFromList(ThrottleClass *throttle);
};

extern ThrottleClass Throttles;

#endif