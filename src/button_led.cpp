#include "button_led.h"
#include <FastLED.h>

// Static member initialization
uint32_t ButtonLED::_lastBreathUpdate = 0;
uint8_t ButtonLED::_breathPhase = 0;
uint32_t ButtonLED::_lastAttentionPulse = 0;
bool ButtonLED::_inAttentionPulse = false;
uint8_t ButtonLED::_attentionPhase = 0;

void ButtonLED::init() {
    // Configure PWM channels
    ledcSetup(PWM_CHANNEL_LEFT, BUTTON_LED_PWM_FREQ, BUTTON_LED_PWM_RES);
    ledcSetup(PWM_CHANNEL_RIGHT, BUTTON_LED_PWM_FREQ, BUTTON_LED_PWM_RES);

    // Attach pins to channels
    ledcAttachPin(BUTTON_LED_LEFT_PIN, PWM_CHANNEL_LEFT);
    ledcAttachPin(BUTTON_LED_RIGHT_PIN, PWM_CHANNEL_RIGHT);

    // Start with LEDs off
    setOff();

    _lastBreathUpdate = millis();
    _lastAttentionPulse = millis();
}

void ButtonLED::setBrightness(bool isLeft, uint8_t brightness) {
    if (isLeft) {
        ledcWrite(PWM_CHANNEL_LEFT, brightness);
    } else {
        ledcWrite(PWM_CHANNEL_RIGHT, brightness);
    }
}

void ButtonLED::setOff() {
    ledcWrite(PWM_CHANNEL_LEFT, 0);
    ledcWrite(PWM_CHANNEL_RIGHT, 0);
}

// Gameplay: Active zone - bright steady light
void ButtonLED::setActiveZone(bool leftActive, bool rightActive) {
    ledcWrite(PWM_CHANNEL_LEFT, leftActive ? 255 : 0);
    ledcWrite(PWM_CHANNEL_RIGHT, rightActive ? 255 : 0);
}

// Gameplay: Successful hit - quick flash then fade
void ButtonLED::flashHit(bool isLeft) {
    uint8_t channel = isLeft ? PWM_CHANNEL_LEFT : PWM_CHANNEL_RIGHT;

    // Quick bright flash
    ledcWrite(channel, 255);
    delay(50);

    // Fade out
    for (int b = 255; b >= 0; b -= 15) {
        ledcWrite(channel, b);
        delay(10);
    }

    ledcWrite(channel, 0);
}

// Gameplay: Miss - rapid blinks 3x
void ButtonLED::blinkMiss(bool isLeft) {
    uint8_t channel = isLeft ? PWM_CHANNEL_LEFT : PWM_CHANNEL_RIGHT;

    for (int i = 0; i < 3; i++) {
        ledcWrite(channel, 255);
        delay(100);
        ledcWrite(channel, 0);
        delay(80);
    }
}

// Gameplay: Countdown pulse synchronized with LED strip
void ButtonLED::pulseCountdown(uint8_t brightness) {
    ledcWrite(PWM_CHANNEL_LEFT, brightness);
    ledcWrite(PWM_CHANNEL_RIGHT, brightness);
}

// Idle: Breathing pattern - slow fade in/out
void ButtonLED::updateBreathing() {
    if (millis() - _lastBreathUpdate < 20) return;
    _lastBreathUpdate = millis();

    _breathPhase++;

    // Sine wave breathing
    uint8_t brightness = sin8(_breathPhase);
    // Scale down for gentler breathing (max 128)
    brightness = brightness / 2;

    ledcWrite(PWM_CHANNEL_LEFT, brightness);
    ledcWrite(PWM_CHANNEL_RIGHT, brightness);
}

// Idle: Attention-grabbing occasional bright pulse
void ButtonLED::triggerAttentionPulse() {
    // Trigger pulse every 8-12 seconds randomly
    if (!_inAttentionPulse && millis() - _lastAttentionPulse > random(8000, 12000)) {
        _inAttentionPulse = true;
        _attentionPhase = 0;
        _lastAttentionPulse = millis();
    }

    if (_inAttentionPulse) {
        _attentionPhase += 8;

        // Quick rise and fall
        uint8_t brightness;
        if (_attentionPhase < 128) {
            brightness = _attentionPhase * 2;  // Rise
        } else {
            brightness = (255 - _attentionPhase) * 2;  // Fall
        }

        ledcWrite(PWM_CHANNEL_LEFT, brightness);
        ledcWrite(PWM_CHANNEL_RIGHT, brightness);

        if (_attentionPhase >= 255) {
            _inAttentionPulse = false;
        }
    }
}
