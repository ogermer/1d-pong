#include "animation.h"

class OceanWaveAnimation : public Animation {
public:
    OceanWaveAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        _time = 0;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 30) return;
        _lastUpdate = millis();

        _time++;

        for (int i = 0; i < numLeds; i++) {
            // Multiple overlapping sine waves at different frequencies
            float wave1 = sin8(_time * 2 + i * 8) / 255.0f;
            float wave2 = sin8(_time * 3 + i * 12 + 64) / 255.0f;
            float wave3 = sin8(_time + i * 5 + 128) / 255.0f;

            // Combine waves
            float combined = (wave1 * 0.5f + wave2 * 0.3f + wave3 * 0.2f);

            // Map to ocean colors (deep blue to cyan)
            uint8_t blue = 80 + combined * 175;
            uint8_t green = 20 + combined * 100;
            uint8_t red = combined * 30;

            // Occasional white foam on peaks
            if (combined > 0.85f) {
                // Foam sparkle
                uint8_t foam = (combined - 0.85f) * 1700;
                red = qadd8(red, foam);
                green = qadd8(green, foam);
                blue = qadd8(blue, foam);
            }

            leds[i] = CRGB(red, green, blue);
        }

        FastLED.show();
    }

private:
    uint32_t _lastUpdate = 0;
    uint16_t _time = 0;
};

REGISTER_ANIMATION(OceanWaveAnimation, "Ocean Wave");
