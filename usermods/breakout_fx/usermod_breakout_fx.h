#pragma once

#include "wled.h"

//
// Palette Sweep: sweeps the entire palette across the segment.
// Speed controls animation speed, Intensity (Scale) controls palette stretch.
// check1 enables ping-pong mode: alternates between normal and reversed palette.
//

static uint16_t mode_palette_sweep(void) {
  if (SEGLEN <= 1) { SEGMENT.fill(SEGCOLOR(0)); return FRAMETIME; }

  // Calculate animation position based on speed
  uint32_t counter = strip.now * ((SEGMENT.speed >> 2) + 1);

  // Ping-pong mode: sweep palette, then sweep reversed palette
  bool pingPong = SEGMENT.check1;
  bool reversePalette = false;
  uint16_t offset;

  if (pingPong) {
    // Continuous sweep but reverse palette colors on alternating cycles
    uint16_t phase = (counter >> 8) & 0x1FF; // 0-511 range for full cycle
    offset = phase & 0xFF; // 0-255 offset, always increasing within each half-cycle
    reversePalette = (phase >= 256); // reverse palette in second half of cycle
  } else {
    // Normal mode: continuous sweep in one direction
    offset = (counter >> 8) & 0xFF; // 0-255 range
  }

  // Intensity controls palette scale:
  // Center (128) = 1x full palette. Lower values = more repetitions (up to ~4x at 0).
  // Higher values = more stretched (down to ~1/4 scale at 255).
  uint16_t paletteScale;
  if (SEGMENT.intensity >= 128) {
    // Map 128-255 intensity to paletteScale 256-66 (1x to ~1/4 scale = more stretched)
    paletteScale = 256 - (((SEGMENT.intensity - 128) * 192) >> 7);
  } else {
    // Map 0-127 intensity to paletteScale 256-1018 (1x to ~4x repetitions)
    paletteScale = 1024 - (((128 - SEGMENT.intensity) * 768) >> 7);
  }

  // Sweep the palette across the segment
  for (unsigned i = 0; i < SEGLEN; i++) {
    uint16_t paletteIndex = ((i * paletteScale) / SEGLEN + offset) & 0xFF;

    if (reversePalette) {
      paletteIndex = 255 - paletteIndex;
    }

    uint32_t color = SEGMENT.color_from_palette(paletteIndex, false, true, 0);
    SEGMENT.setPixelColor(i, color);
  }

  return FRAMETIME;
}
static const char _data_FX_MODE_PALETTE_SWEEP[] PROGMEM = "Palette Sweep@!,Scale,,,,Ping-pong;;!";


/////////////////////
//  UserMod Class  //
/////////////////////

class BreakoutFxUsermod : public Usermod {
 public:
  void setup() {
    // use id=255 for auto-assignment
    strip.addEffect(255, &mode_palette_sweep, _data_FX_MODE_PALETTE_SWEEP);
  }

  void loop() {} // nothing to do in the loop
  uint16_t getId() { return USERMOD_ID_BREAKOUT_FX; }
};
