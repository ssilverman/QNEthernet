#include <Arduino.h>

extern "C" {
#include <lwip/init.h>
}

void setup() {
  lwip_init();
}

void loop() {
  // put your main code here, to run repeatedly:
}
