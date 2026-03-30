#include "wled.h"

// for information how FX metadata strings work see https://kno.wled.ge/interfaces/json-api/#effect-metadata

// static effect, used if an effect fails to initialize
static void mode_static(void) {
  SEGMENT.fill(SEGCOLOR(0));
}

#define FX_FALLBACK_STATIC { mode_static(); return; }

/////////////////////////////
//  Breakout FX functions  //
/////////////////////////////

/*
 * Sweeps the entire palette across the segment.
 * Speed controls animation speed, Intensity (Scale) controls palette stretch.
 * check1 enables ping-pong mode: alternates between normal and reversed palette.
 */
static void mode_palette_sweep(void)
{
  if (SEGLEN <= 1) FX_FALLBACK_STATIC;

  // Calculate animation position based on speed
  // Use 32-bit counter for smoother animation
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
    // Map LED position to palette index, scaled by paletteScale
    // Use uint16_t for intermediate calculation to prevent overflow before masking
    uint16_t paletteIndex = ((i * paletteScale) / SEGLEN + offset) & 0xFF;
    
    // Reverse palette if in ping-pong mode during second half of cycle
    if (reversePalette) {
      paletteIndex = 255 - paletteIndex;
    }
    
    // Get color from palette at this index
    uint32_t color = SEGMENT.color_from_palette(paletteIndex, false, true, 0);
    
    SEGMENT.setPixelColor(i, color);
  }
}
static const char _data_FX_MODE_PALETTE_SWEEP[] PROGMEM = "Palette Sweep@!,Scale,,,,Ping-pong;;!";


/////////////////////
//  UserMod Class  //
/////////////////////

class BreakoutFxUsermod : public Usermod {
 public:
  void setup() override {
    // use id=255 for auto-assignment (the final id is assigned when adding the effect)
    strip.addEffect(255, &mode_palette_sweep, _data_FX_MODE_PALETTE_SWEEP);

    ////////////////////////////////////////
    //  add your effect function(s) here  //
    ////////////////////////////////////////

    // strip.addEffect(255, &mode_your_effect, _data_FX_MODE_YOUR_EFFECT);
  }

  void loop() override {} // nothing to do in the loop
  uint16_t getId() override { return USERMOD_ID_BREAKOUT_FX; }
};

static BreakoutFxUsermod breakout_fx;
REGISTER_USERMOD(breakout_fx);
