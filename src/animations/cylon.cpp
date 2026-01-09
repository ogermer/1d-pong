#include "animation.h"
#include <math.h>

class CylonAnimation : public Animation {
public:
    CylonAnimation(const char* name) : Animation(name) {
        initGaussian();
    }

    void reset() override {
        _pos = NUM_LEDS / 2;
        _dir = 1;
        _lastUpdate = 0;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 55) return;
        _lastUpdate = millis();

        fill_solid(leds, numLeds, CRGB::Black);

        const uint8_t maxBright = 255;

        // Draw eye with Gaussian glow
        for (int i = -6; i <= 6; i++) {
            int p = _pos + i;
            if (p >= 0 && p < numLeds) {
                uint8_t b = (_intensity[i + 6] * maxBright) >> 8;
                leds[p] = CRGB(255, b / 3, 0);  // orange-red
            }
        }

        // Full eye leaves strip before reversing
        if (_pos + 6 < 0) {
            _pos = -6;
            _dir = 1;
        } else if (_pos - 6 >= numLeds) {
            _pos = numLeds + 6;
            _dir = -1;
        }

        _pos += _dir;
        FastLED.show();
    }

private:
    int _pos = NUM_LEDS / 2;
    int _dir = 1;
    uint32_t _lastUpdate = 0;
    uint8_t _intensity[13];  // Pre-calculated Gaussian weights

    void initGaussian() {
        // Target: at distance 6 -> brightness = 0.05
        const float sigma = 2.45f;

        for (int i = -6; i <= 6; i++) {
            float x = (float)i;
            float value = exp(-(x * x) / (2.0f * sigma * sigma));
            _intensity[i + 6] = (uint8_t)(value * 255.0f + 0.5f);
        }
    }
};

REGISTER_ANIMATION(CylonAnimation, "Cylon");
