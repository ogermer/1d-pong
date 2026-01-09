#include "animation.h"

class FireAnimation : public Animation {
public:
    FireAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        // Initialize heat array
        for (int i = 0; i < NUM_LEDS; i++) {
            _heat[i] = 0;
        }
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 30) return;
        _lastUpdate = millis();

        // Cool down every cell a little
        for (int i = 0; i < numLeds; i++) {
            uint8_t cooling = random(0, ((COOLING * 10) / numLeds) + 2);
            if (cooling > _heat[i]) {
                _heat[i] = 0;
            } else {
                _heat[i] -= cooling;
            }
        }

        // Heat drifts from center outward (fire from both ends)
        for (int i = numLeds / 2; i > 1; i--) {
            _heat[i] = (_heat[i - 1] + _heat[i - 2] + _heat[i - 2]) / 3;
        }
        for (int i = numLeds / 2; i < numLeds - 2; i++) {
            _heat[i] = (_heat[i + 1] + _heat[i + 2] + _heat[i + 2]) / 3;
        }

        // Randomly ignite new sparks at the edges (player zones)
        if (random(255) < SPARKING) {
            int pos = random(0, 3);
            _heat[pos] = qadd8(_heat[pos], random(160, 255));
        }
        if (random(255) < SPARKING) {
            int pos = numLeds - 1 - random(0, 3);
            _heat[pos] = qadd8(_heat[pos], random(160, 255));
        }

        // Map heat to colors
        for (int i = 0; i < numLeds; i++) {
            leds[i] = heatColor(_heat[i]);
        }

        FastLED.show();
    }

private:
    static const uint8_t COOLING = 55;
    static const uint8_t SPARKING = 120;
    uint8_t _heat[NUM_LEDS];
    uint32_t _lastUpdate = 0;

    // Convert heat value to flame color
    CRGB heatColor(uint8_t temperature) {
        uint8_t t192 = scale8_video(temperature, 191);

        uint8_t heatramp = t192 & 0x3F;
        heatramp <<= 2;

        if (t192 > 0x80) {
            // Hottest: white-yellow
            return CRGB(255, 255, heatramp);
        } else if (t192 > 0x40) {
            // Middle: yellow-orange
            return CRGB(255, heatramp, 0);
        } else {
            // Coolest: red-orange
            return CRGB(heatramp, 0, 0);
        }
    }
};

REGISTER_ANIMATION(FireAnimation, "Fire");
