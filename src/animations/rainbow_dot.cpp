#include "animation.h"

class RainbowDotAnimation : public Animation {
public:
    RainbowDotAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _hue = 0;
        _pos = 0;
        _dir = 1;
        _lastUpdate = 0;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 40) return;
        _lastUpdate = millis();

        _hue += 2;
        for (uint8_t i = 0; i < numLeds; i++) {
            leds[i] = CHSV(_hue + i * 8, 255, 90);
        }
        leds[_pos] = CRGB::White;

        _pos += _dir;
        if (_pos == 0 || _pos == numLeds - 1) {
            _dir = -_dir;
        }

        FastLED.show();
    }

private:
    uint32_t _hue = 0;
    uint8_t _pos = 0;
    int8_t _dir = 1;
    uint32_t _lastUpdate = 0;
};

REGISTER_ANIMATION(RainbowDotAnimation, "Rainbow Dot");
