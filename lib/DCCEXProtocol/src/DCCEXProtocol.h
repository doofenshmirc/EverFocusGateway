/* -*- c++ -*-
 *
 *
 * Copyright © 2024 Peter Akers
 * Copyright © 2024 Peter Cole
 * Copyright © 2024 Vincent Hamp
 * Copyright © 2023 Peter Akers
 * Copyright © 2023 Peter Cole
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 * This library is aimed at making things easier for throttle developers writing software for
 * Arduino based hardware throttles that wish to use DCC-EX EX-CommandStation native API
 * commands.
 *
 * For more information, refer to the included README file, and the DCC-EX website.
 * https://www.dcc-ex.com
 */

/*
Version information:

1.2.0   - Add loco hand off method handOffLoco(locoAddress, automationId)
        - Add readCV(cv) and validateCV(cv, value) methods with associated delegate method:
                receivedValidateCV(int cv, int value)
        - Add write loco address writeLocoAddress(address) with associated delegate method:
                receivedWriteLoco(int address)
        - Add validateCVBit(cv, bit, value) method with associated delegate method:
                receivedValidateCVBit(int cv, int bit, int value)
        - Add writeCV(cv, value) with delegate method receivedWriteCV(int cv, int value)
        - Add writeCVBit(cv, bit, value) - note there is no response for this due to parser limitations
        - Add writeCVOnMain(address, cv, value)
        - Add writeCVBitOnMain(address, cv, bit, value)
1.1.0   - Add new track power methods:
        - powerMainOn()/powerMainOff() - Control track power for MAIN track only
        - powerProgOn()/powerProgOff() - Control track power for PROG track only
        - joinProg() - Join PROG to MAIN
1.0.2   - No functional changes, updated examples to use receivedLocoBroadcast() for non-roster Loco objects
1.0.1   - Add additional receivedLocoBroadcast() delegate method to cater for non-roster updates
1.0.0   - First Production release
        - Add methods to clear and refresh the various lists
        - Various memory leak bugfixes
        - Fix bug where any Loco created was added to the roster, despite LocoSourceEntry being set
        - Fix bug where getById() for Turnout, Route, and Turntable was not a static method, causing runtime errors
        - Removed redundant count on Turnout, Route, and Turntable as these are available from getRosterCount,
                getTurnoutCount, getRouteCount, getTurntableCount
        - Updated all public methods setting and getting names from char * to const char * to remove compiler warnings
        - Enable configuring the max parameters parsed by DCCEXInbound via the DCCEXProtocol constructor
        - Implemented many new tests
0.0.17  - Fix typo in turntable example
        - Fix bug where the turntable isMoving() method always returned true
        - Add enableHeartbeat(heartbeatDelay) to send a heartbeat every x ms if a command is not sent
0.0.16  - add public sendCommand method
0.0.15  - any acquired loco is now retained in the roster
0.0.14  - add getNumberSupportedLocos()   used for the fake heartbeat
0.0.13  - Fix bug to allow compilation on AVR platforms, change ssize_t to int
        - Add serial connectivity example
        - Add support for SCREEN updates to delegate
        - Enhance buffer management to clear command buffer if full
0.0.12  - Improved memory management
0.0.11  - support for individual track power   receivedIndividualTrackPower(TrackPower state, int track)
        - improved logic for overall track power
0.0.10  - Add support for broadcast messages
0.0.9   - if loco is selected by address and that loco is in the roster (with the same DCC Address), updated and send
          speed commands for both
0.0.8   - No functional changes, add cross-platform and unit testing capabilities (credit to
          higaski)
0.0.7   - Add isFunctionMomentary(int function);
0.0.6   - Add getFunctionName(int function);
0.0.5   - Increase MAX_FUNCTIONS to 32.
        - Also add check to make sure the incoming does not exceed MAX_FUNCTIONS
0.0.4   - No functional changes, update author/maintainer and URL library properties
0.0.3   - Add getByAddress method to ConsistLoco
        - Fix bug when removing locos from a consist
        - Tidy setTrackType() method
0.0.2   - Add TrackManager configuration method and broadcast processing
        - Add TrackManager, SSID, and mDNS examples
0.0.1   - Initial library release via the Arduino Library Manager
*/

#ifndef DCCEXPROTOCOL_H
#define DCCEXPROTOCOL_H

#include "DCCEXInbound.h"
#include "DCCEXLoco.h"
#include "DCCEXRoutes.h"
#include "DCCEXTurnouts.h"
#include "DCCEXTurntables.h"
#include <Arduino.h>

const int MAX_OUTBOUND_COMMAND_LENGTH = 100; // Max number of bytes for outbound commands

// Valid track power state values
enum TrackPower {
  PowerOff = 0,
  PowerOn = 1,
  PowerUnknown = 2,
};

// Valid TrackManager types
enum TrackManagerMode {
  MAIN, // Normal DCC track mode
  PROG, // Programming DCC track mode
  DC,   // DC mode
  DCX,  // Reverse polarity DC mode
  NONE, // Track is unused
};

/// @brief Nullstream class for initial DCCEXProtocol instantiation to direct streams to nothing
class NullStream : public Stream {
public:
  /// @brief Constructor for the NullStream object
  NullStream() {}

  /// @brief Dummy availability check
  /// @return Returns false (0) always
  int available() { return 0; }

  /// @brief Dummy flush method
  void flush() {}

  /// @brief Dummy peek method
  /// @return Returns -1 always
  int peek() { return -1; }

  /// @brief Dummy read method
  /// @return Returns -1 always
  int read() { return -1; }

  /// @brief Dummy write method for single int
  /// @param c Number received
  /// @return Returns 1 always
  size_t write(uint8_t c) { return 1; }

  /// @brief Dummy write method for buffered input
  /// @param buffer Buffer received
  /// @param size Size of buffer
  /// @return Returns size of buffer always
  size_t write(const uint8_t *buffer, size_t size) { return size; }
};

/// @brief Delegate responses and broadcast events to the client software to enable custom event handlers
class DCCEXProtocolDelegate {
public:
  /// @brief Notify when the server version has been received
  /// @param major Major version of EX-CommandStation (eg. 5.0.7 returns 5)
  /// @param minor Minor version of EX-CommandStation (eg. 5.0.7 returns 0)
  /// @param patch Patch version of EX-CommandStation (eg. 5.0.7 returns 7)
  virtual void receivedServerVersion(int major, int minor, int patch) {}

  /// @brief Notify when a broadcast message has been received
  /// @param message message that has been broadcast
  virtual void receivedMessage(char *message) {}

  /// @brief Notify when the roster list is received
  virtual void receivedRosterList() {}

  /// @brief Notify when the turnout list is received
  virtual void receivedTurnoutList() {}

  /// @brief Notify when the route list is received
  virtual void receivedRouteList() {}

  /// @brief Notify when the turntable list is received
  virtual void receivedTurntableList() {}

  /// @brief Notify when an update to a Loco object is received
  /// @param loco Pointer to the loco object
  virtual void receivedLocoUpdate(Loco *loco) {}

  /// @brief Notify when a Loco broadcast has been received - suitable for non-roster locos
  /// @param address DCC address of the loco
  /// @param speed Speed as derived from the speed byte
  /// @param direction Direction as derived from the speed byte
  /// @param functionMap Function map
  virtual void receivedLocoBroadcast(int address, int speed, Direction direction, int functionMap) {}

  /// @brief Notify when the global track power state change is received
  /// @param state Power state received (PowerOff|PowerOn|PowerUnknown)
  virtual void receivedTrackPower(TrackPower state) {}

  /// @brief Notify when an individual track power state change is received
  /// @param state Power state received (PowerOff|PowerOn|PowerUnknown)
  /// @param track which track changed 65=A..72=H | 2698315=MAIN | 2788330=PROG | 2183=DC | 71999=DCX
  virtual void receivedIndividualTrackPower(TrackPower state, int track) {}

  /// @brief Notify when a track type change is received
  /// @param track track that changed
  /// @param type type received (MAIN|PROG|DC|DCX|NONE)
  /// @param address address received for DC and DCX (zero if other types)
  virtual void receivedTrackType(char track, TrackManagerMode type, int address) {}

  /// @brief Notify when a turnout state change is received
  /// @param turnoutId ID of the turnout
  /// @param thrown Wether it is thrown or not (true|false)
  virtual void receivedTurnoutAction(int turnoutId, bool thrown) {}

  /// @brief Notify when a sensor state change is received
  /// @param address ID of the sensor
  /// @param state Wether it is high or low (true|false)
  virtual void receivedSensorState(int address, bool state);

  /// @brief Notify when a turntable index change is received
  /// @param turntableId ID of the turntable
  /// @param position Index of the position it is moving (or has moved) to
  /// @param moving Whether it is moving or not (true|false)
  virtual void receivedTurntableAction(int turntableId, int position, bool moving) {}

  /// @brief Notify when a loco address is read from the programming track
  /// @param address DCC address read from the programming track, or -1 for a failure to read
  virtual void receivedReadLoco(int address) {}

  /// @brief Notify when a CV is read or validated from the programming track
  /// @param cv CV the value has been read from
  /// @param value Value read from the CV, or -1 for a failure to read
  virtual void receivedValidateCV(int cv, int value) {}

  /// @brief Notify when a CV bit is validated from the programming track
  /// @param cv CV the bit is being validated in
  /// @param bit Bit of the CV being validated
  /// @param value Value validated from the bit, or -1 if not valid
  virtual void receivedValidateCVBit(int cv, int bit, int value) {}

  /// @brief Notify when a Loco address has been written on the programming track
  /// @param address DCC address written to the loco, or -1 for a failure to write
  virtual void receivedWriteLoco(int address) {}

  /// @brief Notify when a CV is written on the programming track
  /// @param cv CV being written to
  /// @param value Value written, or -1 for failure
  virtual void receivedWriteCV(int cv, int value) {}

  /// @brief Notify when a screen update is received
  /// @param screen Screen number
  /// @param row Row number
  /// @param message Message to display on the screen/row
  virtual void receivedScreenUpdate(int screen, int row, char *message) {}
};

/// @brief Main class for the DCCEXProtocol library
class DCCEXProtocol {
public:
  // Protocol and server methods

  /// @brief Constructor for the DCCEXProtocol object
  /// @param maxCmdBuffer Optional - maximum number of bytes for the command buffer (default 500)
  /// @param maxCommandParams Optional - maximum number of parameters to parse via the DCCEXInbound parser (default 50)
  DCCEXProtocol(int maxCmdBuffer = 500, int maxCommandParams = 50);

  /// @brief Destructor for the DCCEXProtocol object
  ~DCCEXProtocol();

  /// @brief Set the delegate object for callbacks
  /// @param delegate
  void setDelegate(DCCEXProtocolDelegate *delegate);

  /// @brief Set the stream object for console output
  /// @param console
  void setLogStream(Stream *console);

  /// @brief Enable heartbeat if required - can help WiFi connections that drop out
  /// @param heartbeatDelay Time in milliseconds between heartbeats - defaults to one minute (60000ms)
  void enableHeartbeat(unsigned long heartbeatDelay = 60000);

  /// @brief Connect the stream object to interact with DCC-EX
  /// @param stream
  void connect(Stream *stream);

  /// @brief Disconnect from DCC-EX
  void disconnect();

  /// @brief Check for incoming DCC-EX broadcasts/responses and parse them
  void check();

  /// @brief allows sending of an arbitray command
  /// @param cmd Command to send
  void sendCommand(char *cmd);

  /// @brief Request DCC-EX object lists (Roster, Turnouts, Routes, Turntables)
  /// @param rosterRequired Request the roster list (true|false)
  /// @param turnoutListRequired Request the turnout list (true|false)
  /// @param routeListRequired Request the route list (true|false)
  /// @param turntableListRequired Request the turntable list (true|false)
  void getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);

  /// @brief Check if all lists have been received (roster, routes, turnouts, turntables)
  /// @return true|false
  bool receivedLists();

  /// @brief Request server version information
  void requestServerVersion();

  /// @brief Check if server version has been received
  /// @return
  bool receivedVersion();

  /// @brief Retrieve the major version of EX-CommandStation
  /// @return Major version number eg. 5.y.z
  int getMajorVersion();

  /// @brief Retrieve the minor version of EX-Commandstation
  /// @return Minor version number eg. x.0.z
  int getMinorVersion();

  /// @brief Retreive the patch version of EX-CommandStation
  /// @return Patch version number eg. x.y.7
  int getPatchVersion();

  /// @brief Retrieve the last time the server responded
  /// @return Last response time in milliseconds (from millis())
  unsigned long getLastServerResponseTime(); // seconds since Arduino start

  /// @brief Clear roster, turnout, turntable, and route lists
  void clearAllLists();

  /// @brief Clear roster, turnout, turntable, and route lists and request new ones
  void refreshAllLists();

  // Consist/Loco methods

  /// @brief Set the provided loco to the specified speed and direction
  /// @param loco Pointer to a Loco object
  /// @param speed Speed (0 - 126)
  /// @param direction Direction (Forward|Reverse)
  void setThrottle(Loco *loco, int speed, Direction direction);

  /// @brief Set all locos in the provided consist to the specified speed and direction
  /// @param consist Pointer to a consist object
  /// @param speed Speed (0 - 126)
  /// @param direction Direction (Forward|Reverse) - reverse facing locos will be adjusted automatically
  void setThrottle(Consist *consist, int speed, Direction direction);

  /// @brief Turn the specified function on for the provided loco
  /// @param loco Pointer to a loco object
  /// @param function Function number (0 - 27)
  void functionOn(Loco *loco, int function);

  /// @brief Turn the specified function off for the provided loco
  /// @param loco Pointer to a loco object
  /// @param function Function number (0 - 27)
  void functionOff(Loco *loco, int function);

  /// @brief Test if the specified function for the provided loco is on
  /// @param loco Pointer to a loco object
  /// @param function Function number to test (0 - 27)
  /// @return true = on, false = off
  bool isFunctionOn(Loco *loco, int function);

  /// @brief Turn the specified function on for the provided consist
  /// @param consist Pointer to a consist object
  /// @param function Function number (0 - 27)
  void functionOn(Consist *consist, int function);

  /// @brief Turn the specified function off for the provided consist
  /// @param consist Pointer to a consist object
  /// @param function Function number (0 - 27)
  void functionOff(Consist *consist, int function);

  /// @brief Test if the specified function for the provided consist is on (Checks first loco)
  /// @param consist Pointer to a consist object
  /// @param function Function number to test (0 - 27)
  /// @return true = on, false = off
  bool isFunctionOn(Consist *consist, int function);

  /// @brief Explicitly request an update for the specified loco
  /// @param address DCC address of the loco
  void requestLocoUpdate(int address);

  /// @brief Initiate reading a loco address from the programming track, response will be a delegate notification
  void readLoco();

  /// @brief Initiate an emergency stop
  void emergencyStop();

  // Roster methods

  /// @brief Get the number of roster entries
  /// @return Number of roster entries received
  int getRosterCount();

  /// @brief Check if roster has been received
  /// @return true|false
  bool receivedRoster();

  /// @brief Search for the specified DCC address in the roster
  /// @param address DCC address to search for
  /// @return Pointer to the Loco object
  Loco *findLocoInRoster(int address);

  /// @brief Clear the roster
  void clearRoster();

  /// @brief Clear the roster and request again
  void refreshRoster();

  // Turnout methods

  /// @brief Get the number of turnouts
  /// @return Number of turnouts received
  int getTurnoutCount();

  /// @brief Check if turnout list has been received
  /// @return true|false
  bool receivedTurnoutList();

  /// @brief Retrieve a turnout/point object by its ID
  /// @param turnoutId ID of the turnout/point
  /// @return The turnout/point object
  Turnout *getTurnoutById(int turnoutId);

  /// @brief Close the specified turnout/point
  /// @param turnoutId ID of the turnout/point
  void closeTurnout(int turnoutId);

  /// @brief Throw the specified turnout/point
  /// @param turnoutId ID of the turnout/point
  void throwTurnout(int turnoutId);

  /// @brief Toggle the specified turnout/point (if closed, will throw, and vice versa)
  /// @param turnoutId ID of the turnout/point
  void toggleTurnout(int turnoutId);

  /// @brief Clear the list of turnouts
  void clearTurnoutList();

  /// @brief Clear the list of turnouts and request again
  void refreshTurnoutList();

  // Route methods

  /// @brief Get the number of route entries
  /// @return Number of routes received
  int getRouteCount();

  /// @brief Check if route list has been received
  /// @return true|false
  bool receivedRouteList();

  /// @brief Start a route/automation
  /// @param routeId ID of the route/automation to start
  void startRoute(int routeId);

  /// @brief Hand off a Loco to an Automation - note you must retrieve the ROUTE list to use this
  /// @param locoAddress DCC address of the Loco to hand off
  /// @param automationId ID of the automation to start, must be RouteType::RouteTypeAutomation
  void handOffLoco(int locoAddress, int automationId);

  /// @brief Pause all routes/automations
  void pauseRoutes();

  /// @brief Resume all routes/automations
  void resumeRoutes();

  /// @brief Clear all routes
  void clearRouteList();

  /// @brief Clear all routes and request a new list
  void refreshRouteList();

  // Turntable methods

  /// @brief Get the number of turntable entries
  /// @return Number of turntables received
  int getTurntableCount();

  /// @brief Check if turntable list has been received
  /// @return true|false
  bool receivedTurntableList();

  /// @brief Retrieve a turntable object by its ID
  /// @param turntableId ID of the turntable
  /// @return The turntable object
  Turntable *getTurntableById(int turntableId);

  /// @brief Rotate a turntable object
  /// @param turntableId ID of the turntable
  /// @param position Position index to rotate to
  /// @param activity Optional activity for EX-Turntable objects only
  void rotateTurntable(int turntableId, int position, int activity = 0);

  /// @brief Clear all turntables
  void clearTurntableList();

  /// @brief Clear all turntables and request a new list
  void refreshTurntableList();

  // Track management methods

  /// @brief Global track power on command
  void powerOn();

  /// @brief Global track power off command
  void powerOff();

  /// @brief Main track power on command
  void powerMainOn();

  /// @brief Main track power off command
  void powerMainOff();

  /// @brief Prog track power on command
  void powerProgOn();

  /// @brief Prog track power off command
  void powerProgOff();

  /// @brief Join Prog track to Main
  void joinProg();

  /// @brief Turn power on for the specified track
  /// @param track Track name (A - H)
  void powerTrackOn(char track);

  /// @brief Turn power off for the specified track
  /// @param track Track name (A - H)
  void powerTrackOff(char track);

  /// @brief set track type for the specified track
  /// @param track Track name (A - H)
  /// @param type Track type (MAIN, PROG, DC, DCX, NONE)
  /// @param address dcc address for DC and DCX  (Required, but ignored if not DC or DCX)
  void setTrackType(char track, TrackManagerMode type, int address);

  // DCC accessory methods

  /// @brief Activate DCC accessory at the specified address and subaddress
  /// @param accessoryAddress Address of the DCC accessory
  /// @param accessorySubAddr Subaddress of the DCC accessory
  void activateAccessory(int accessoryAddress, int accessorySubAddr);

  /// @brief Deactivate DCC accessory at the specified address and subaddress
  /// @param accessoryAddress Address of the DCC accessory
  /// @param accessorySubAddr Subaddress of the DCC accessory
  void deactivateAccessory(int accessoryAddress, int accessorySubAddr);

  /// @brief Activate DCC accessory at the specified linear address
  /// @param linearAddress Linear address of the DCC accessory
  void activateLinearAccessory(int linearAddress);

  /// @brief Deactivate DCC accessory at the specified linear address
  /// @param linearAddress Linear address of the DCC accessory
  void deactivateLinearAccessory(int linearAddress);

  /// @brief Request the number of supported cabs(locos)
  void getNumberSupportedLocos();

  // CV programming methods

  /// @brief Read the value of the provided CV from the Loco on the programming track
  /// @param cv CV number to read the value of
  void readCV(int cv);

  /// @brief Validate the provided value is stored in the provided CV
  /// @param cv CV number to validate the value of
  /// @param value Value to validate
  void validateCV(int cv, int value);

  /// @brief Validate the provided bit is set to the specified value for the provided CV
  /// @param cv CV number to validate the bit of
  /// @param bit Bit for the CV to validate
  /// @param value Value to validate (0|1)
  void validateCVBit(int cv, int bit, int value);

  /// @brief Write Loco address to the Loco on the programming track
  /// @param address DCC address to write
  void writeLocoAddress(int address);

  /// @brief Write the provided value to the specified CV on the programming track
  /// @param cv CV number to write to
  /// @param value Value to write to the CV
  void writeCV(int cv, int value);

  /// @brief Write the provided value to the specified bit and CV on the programming track.
  /// Note that the response is in legacy DCC++ formatting and is not supported by the DCCEXInbound parser.
  /// Therefore, the throttle will not receive a response. We suggest writing full CVs.
  /// @param cv CV number to write to
  /// @param bit Bit for the CV to write
  /// @param value Value to write (0|1)
  void writeCVBit(int cv, int bit, int value);

  /// @brief Write the provided value to the specified Loco address and CV on the main track
  /// @param address DCC address of the Loco
  /// @param cv CV number to write to
  /// @param value Value to write to the CV
  void writeCVOnMain(int address, int cv, int value);

  /// @brief Write the provided value to the specified Loco, CV, and bit on the main track
  /// @param address DCC address of the Loco
  /// @param cv CV number to write to
  /// @param bit Bit for the CV to write
  /// @param value Value to write (0|1)
  void writeCVBitOnMain(int address, int cv, int bit, int value);

  // Attributes

  /// @brief Linked list of Loco objects to form the roster
  Loco *roster = nullptr;

  /// @brief Linked list of Turnout objects to form the turnout list
  Turnout *turnouts = nullptr;

  /// @brief Linked list of Route objects to form the list of routes and automations
  Route *routes = nullptr;

  /// @brief Linked list of Turntable objects to form the list of turntables
  Turntable *turntables = nullptr;

private:
  // Methods
  // Protocol and server methods
  void _init();
  void _sendCommand();
  void _processCommand();
  void _processServerDescription();
  void _processMessage();
  void _processScreenUpdate();
  void _sendHeartbeat();

  // Consist/loco methods
  void _processLocoBroadcast();
  int _getValidFunctionMap(int functionMap);
  int _getSpeedFromSpeedByte(int speedByte);
  Direction _getDirectionFromSpeedByte(int speedByte);
  void _setLoco(int address, int speed, Direction direction);
  void _processReadResponse();

  // Roster methods
  void _getRoster();
  bool _requestedRoster();
  void _processRosterList();
  void _requestRosterEntry(int address);
  void _processRosterEntry();

  // Turnout methods
  void _getTurnouts();
  bool _requestedTurnouts();
  void _processTurnoutList();
  void _requestTurnoutEntry(int id);
  void _processTurnoutEntry();
  void _processTurnoutBroadcast();

  // Route methods
  void _getRoutes();
  bool _requestedRoutes();
  void _processRouteList();
  void _requestRouteEntry(int id);
  void _processRouteEntry();

  // Turntable methods
  void _getTurntables();
  bool _requestedTurntables();
  void _processTurntableList();
  void _requestTurntableEntry(int id);
  void _processTurntableEntry();
  void _requestTurntableIndexEntry(int id);
  void _processTurntableIndexEntry();
  void _processTurntableBroadcast();

  // Track management methods
  void _processTrackPower();
  void _processTrackType();

  // CV programming methods
  void _processValidateCVResponse();
  void _processValidateCVBitResponse();
  void _processWriteLocoResponse();
  void _processWriteCVResponse();

  // Attributes
  int _rosterCount = 0;                               // Count of roster items received
  int _turnoutCount = 0;                              // Count of turnout objects received
  int _routeCount = 0;                                // Count of route objects received
  int _turntableCount = 0;                            // Count of turntable objects received
  int _version[3] = {};                               // EX-CommandStation version x.y.z
  Stream *_stream;                                    // Stream object where commands are sent/received
  Stream *_console;                                   // Stream object for console output
  NullStream _nullStream;                             // Send streams to null if no object provided
  int _bufflen;                                       // Used to ensure command buffer size not exceeded
  int _maxCmdBuffer;                                  // Max size for the command buffer
  char *_cmdBuffer;                                   // Char array for inbound command buffer
  char _outboundCommand[MAX_OUTBOUND_COMMAND_LENGTH]; // Char array for outbound commands
  DCCEXProtocolDelegate *_delegate = nullptr;         // Pointer to the delegate for notifications
  unsigned long _lastServerResponseTime;              // Records the timestamp of the last server response
  char _inputBuffer[512];                             // Char array for input buffer
  int _nextChar;                                      // where the next character to be read goes in the buffer
  bool _receivedVersion = false;                      // Flag that server version has been received
  bool _receivedLists = false;                        // Flag if all requested lists have been received
  bool _rosterRequested = false;                      // Flag that roster has been requested
  bool _receivedRoster = false;                       // Flag that roster has been received
  bool _turnoutListRequested = false;                 // Flag that turnout list requested
  bool _receivedTurnoutList = false;                  // Flag that turnout list received
  bool _routeListRequested = false;                   // Flag that route list requested
  bool _receivedRouteList = false;                    // Flag that route list received
  bool _turntableListRequested = false;               // Flag that turntable list requested
  bool _receivedTurntableList = false;                // Flag that turntable list received
  bool _enableHeartbeat;                              // Flag if heartbeat is enabled
  unsigned long _heartbeatDelay;                      // Delay between heartbeats if enabled
  unsigned long _lastHeartbeat;                       // Time in ms of the last heartbeat, also set by sending a command
};

#endif // DCCEXPROTOCOL_H
