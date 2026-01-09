#include "animation.h"

class PlasmaCometAnimation : public Animation {
public:
    PlasmaCometAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _hue = 0;
        _cometPos = NUM_LEDS / 2;
        _cometDir = 1;
        _lastUpdate = 0;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 45) return;
        _lastUpdate = millis();

        _hue += 1;

        // Draw plasma background
        for (uint8_t i = 0; i < numLeds; i++) {
            uint8_t h = _hue + i * 10 + sin8(millis() / 30 + i * 6);
            leds[i] = CHSV(h, 220, 90);
        }

        // Draw comet with trail
        for (int t = 0; t < 8; t++) {
            int p = _cometPos - t * _cometDir;
            if (p >= 0 && p < numLeds) {
                uint8_t brightness = 255 - t * 32;
                leds[p] = CRGB(brightness, brightness, brightness);
            }
        }

        // Move comet
        _cometPos += _cometDir;
        if (_cometPos < 0) {
            _cometPos = 0;
            _cometDir = 1;
        }
        if (_cometPos >= numLeds) {
            _cometPos = numLeds - 1;
            _cometDir = -1;
        }

        FastLED.show();
    }

private:
    uint32_t _hue = 0;
    int _cometPos = NUM_LEDS / 2;
    int _cometDir = 1;
    uint32_t _lastUpdate = 0;
};

REGISTER_ANIMATION(PlasmaCometAnimation, "Plasma Comet");
