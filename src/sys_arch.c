// Provide system function implememtations.
// (c) 2021 Shawn Silverman

#include <Arduino.h>
#include <lwip/arch.h>

u32_t sys_now(void) {
  return millis();
}
