#pragma once

// ======================================================
// Hardware Configuration
// ======================================================
#define LED_PIN             5
#define NUM_LEDS            55
#define LED_TYPE            WS2812B
#define COLOR_ORDER         GRB
#define BUTTON_LEFT_PIN     17
#define BUTTON_RIGHT_PIN    18
#define BUTTON_ACTIVE_LEVEL LOW

// ======================================================
// Global Brightness
// ======================================================
#define BRIGHTNESS          255

// ======================================================
// Animation Configuration
// ======================================================
#define ANIMATION_DURATION_MS  10000UL    // Duration each animation plays before switching

// ======================================================
// Game Parameters
// ======================================================
#define ZONE_SIZE_START     10
#define ZONE_SIZE_MIN       5
#define SCORE_TO_WIN        5

// ======================================================
// Ball Speed Control
// ======================================================
#define BALL_INITIAL_DELAY_MS  60    // Main tuning knob for ball speed

// Derived values (scale automatically from BALL_INITIAL_DELAY_MS)
#define BALL_DELAY_START        BALL_INITIAL_DELAY_MS
#define BALL_DELAY_MIN          (BALL_INITIAL_DELAY_MS / 5)
#define BALL_SPEEDUP_PER_RETURN (BALL_INITIAL_DELAY_MS / 14)

// Early hit bonus: hitting the ball as it enters the zone gives extra speedup
// Max bonus when ball just entered zone, 0 bonus when ball is about to exit
#define BALL_EARLY_HIT_MAX_BONUS (BALL_INITIAL_DELAY_MS / 8)  // ~7ms bonus at 60ms base

// ======================================================
// Colors
// ======================================================
#define COLOR_BACKGROUND    CRGB::Black
#define COLOR_BALL          CRGB::White
#define COLOR_ZONE_LEFT     CRGB::Blue
#define COLOR_ZONE_RIGHT    CRGB::Green
#define COLOR_MISS          CRGB::Red
#define COLOR_WIN_LEFT      CRGB::Blue
#define COLOR_WIN_RIGHT     CRGB::Green
