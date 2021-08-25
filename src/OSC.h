// OSC function declarations.
// (c) 2021 Shawn Silverman

#ifndef OSC_H_
#define OSC_H_

#include <Print.h>

// Print an OSC message, either a bundle or not.
void printOSC(Print &out, const uint8_t *b, int len);

#endif  // OSC_H_
