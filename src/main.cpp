#include "Arduino.h"
#include "CommandStation.h"
#include "LocoNetInterface.h"
#include "DCCEXInterface.h"

void setup() {
  DCCEX_LOG.begin(115200);
  DCCEX_STREAM.begin(115200);
  
  CommandStation.init();
  LocoNetInterface.init(LOCONET_TX);
  DCCEXInterface.init(&DCCEX_STREAM, &DCCEX_LOG);

  DIAG("%s\n", CS_NAME);
}

void loop() {
  LocoNetInterface.check();
  DCCEXInterface.check();
  CommandStation.check();
}