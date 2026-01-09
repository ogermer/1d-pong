#include "animation.h"

class HeartbeatAnimation : public Animation {
public:
    HeartbeatAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        _phase = 0;
        _brightness = 0;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 10) return;
        _lastUpdate = millis();

        _phase++;

        // Heartbeat timing pattern (lub-dub pause)
        // Total cycle ~1200ms for ~50bpm feel
        uint8_t targetBrightness = 0;

        if (_phase < 15) {
            // First beat (lub) - quick rise
            targetBrightness = _phase * 17;
        } else if (_phase < 30) {
            // First beat fall
            targetBrightness = 255 - (_phase - 15) * 12;
        } else if (_phase < 45) {
            // Brief pause
            targetBrightness = 75 - (_phase - 30) * 3;
        } else if (_phase < 60) {
            // Second beat (dub) - slightly softer
            targetBrightness = (_phase - 45) * 14;
        } else if (_phase < 75) {
            // Second beat fall
            targetBrightness = 210 - (_phase - 60) * 14;
        } else if (_phase < 120) {
            // Long pause before next heartbeat
            targetBrightness = 0;
        } else {
            _phase = 0;
            targetBrightness = 0;
        }

        _brightness = targetBrightness;

        // Pulse outward from center
        uint8_t center = numLeds / 2;

        for (int i = 0; i < numLeds; i++) {
            // Distance from center (0 at center, 1 at edges)
            float dist = (float)abs(i - center) / (float)center;

            // Brightness falls off toward edges
            uint8_t ledBrightness = _brightness * (1.0f - dist * 0.5f);

            // Deep red color for heartbeat
            leds[i] = CRGB(ledBrightness, ledBrightness / 8, ledBrightness / 10);
        }

        FastLED.show();
    }

private:
    uint32_t _lastUpdate = 0;
    uint16_t _phase = 0;
    uint8_t _brightness = 0;
};

REGISTER_ANIMATION(HeartbeatAnimation, "Heartbeat");
