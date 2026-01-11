#pragma once

#include <Arduino.h>
#include "config.h"

// PWM channels for button LEDs
#define PWM_CHANNEL_LEFT  0
#define PWM_CHANNEL_RIGHT 1

class ButtonLED {
public:
    static void init();

    // Gameplay effects
    static void setActiveZone(bool leftActive, bool rightActive);
    static void flashHit(bool isLeft);
    static void blinkMiss(bool isLeft);
    static void pulseCountdown(uint8_t brightness);

    // Idle mode effects
    static void updateBreathing();
    static void triggerAttentionPulse();

    // Direct control
    static void setBrightness(bool isLeft, uint8_t brightness);
    static void setOff();

private:
    static uint32_t _lastBreathUpdate;
    static uint8_t _breathPhase;
    static uint32_t _lastAttentionPulse;
    static bool _inAttentionPulse;
    static uint8_t _attentionPhase;
};
