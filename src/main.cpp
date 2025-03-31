#include <Arduino.h>

#include "CommandStation.h"
#include "DCCEXDelegate.h"
#include "config.h"


void setup() {
  Serial.begin(115200);
  DCCEX_SERIAL.begin(115200);
  DCCEXDelegate.init(&DCCEX_SERIAL);
  CommandStation.init(7);
  DIAG("%s started\n", PRG_NAME);
}

void loop() {
  CommandStation.check();
  DCCEXDelegate.check();
}