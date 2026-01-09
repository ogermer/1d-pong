#include "animation.h"

class MatrixRainAnimation : public Animation {
public:
    MatrixRainAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        // Initialize drops
        for (int i = 0; i < NUM_DROPS; i++) {
            _dropPos[i] = random(0, NUM_LEDS);
            _dropSpeed[i] = 1 + random(0, 3);
            _dropLength[i] = 3 + random(0, 5);
            _dropDelay[i] = random(0, 100);
        }
        // Clear brightness array
        for (int i = 0; i < NUM_LEDS; i++) {
            _brightness[i] = 0;
        }
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 40) return;
        _lastUpdate = millis();

        // Fade existing pixels
        for (int i = 0; i < numLeds; i++) {
            _brightness[i] = qsub8(_brightness[i], 25);
        }

        // Update each drop
        for (int d = 0; d < NUM_DROPS; d++) {
            if (_dropDelay[d] > 0) {
                _dropDelay[d]--;
                continue;
            }

            // Draw drop with trail
            for (int t = 0; t < _dropLength[d]; t++) {
                int pos = _dropPos[d] - t;
                if (pos >= 0 && pos < numLeds) {
                    uint8_t bright = 255 - (t * (200 / _dropLength[d]));
                    if (bright > _brightness[pos]) {
                        _brightness[pos] = bright;
                    }
                }
            }

            // Move drop
            _dropPos[d] += _dropSpeed[d];

            // Reset drop when it exits
            if (_dropPos[d] - _dropLength[d] >= numLeds) {
                _dropPos[d] = 0;
                _dropSpeed[d] = 1 + random(0, 3);
                _dropLength[d] = 3 + random(0, 5);
                _dropDelay[d] = random(0, 50);
            }
        }

        // Occasional bright glitch
        if (random(100) < 3) {
            int pos = random(numLeds);
            _brightness[pos] = 255;
        }

        // Render to LEDs (matrix green)
        for (int i = 0; i < numLeds; i++) {
            uint8_t g = _brightness[i];
            uint8_t r = g / 8;  // Slight red tint for white-ish heads
            leds[i] = CRGB(r, g, r / 2);
        }

        FastLED.show();
    }

private:
    static const int NUM_DROPS = 6;
    int _dropPos[NUM_DROPS];
    int _dropSpeed[NUM_DROPS];
    int _dropLength[NUM_DROPS];
    int _dropDelay[NUM_DROPS];
    uint8_t _brightness[NUM_LEDS];
    uint32_t _lastUpdate = 0;
};

REGISTER_ANIMATION(MatrixRainAnimation, "Matrix Rain");
