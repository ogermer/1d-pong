#include "animation.h"

class LightningAnimation : public Animation {
public:
    LightningAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        _flashBrightness = 0;
        _rumbleBrightness = 0;
        _nextFlash = millis() + random(500, 2000);
        _flashCount = 0;
        _inFlashSequence = false;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 15) return;
        _lastUpdate = millis();

        uint32_t now = millis();

        // Trigger new flash sequence
        if (!_inFlashSequence && now >= _nextFlash) {
            _inFlashSequence = true;
            _flashCount = 1 + random(0, 3);  // 1-3 flashes per sequence
            _flashBrightness = 255;
        }

        // Handle flash sequence
        if (_inFlashSequence) {
            if (_flashBrightness > 0) {
                // Flash is active - rapid decay
                _flashBrightness = qsub8(_flashBrightness, 60);

                if (_flashBrightness == 0 && _flashCount > 0) {
                    // Brief pause then next flash
                    _flashCount--;
                    if (_flashCount > 0) {
                        _flashBrightness = 200 + random(0, 55);
                    } else {
                        // End of sequence, start rumble
                        _inFlashSequence = false;
                        _rumbleBrightness = 120;
                        _nextFlash = now + random(1500, 4000);
                    }
                }
            }
        }

        // Fade rumble (thunder afterglow)
        if (_rumbleBrightness > 0) {
            _rumbleBrightness = qsub8(_rumbleBrightness, 3);
        }

        // Render
        for (int i = 0; i < numLeds; i++) {
            if (_flashBrightness > 0) {
                // Bright white flash
                leds[i] = CRGB(_flashBrightness, _flashBrightness, _flashBrightness);
            } else if (_rumbleBrightness > 0) {
                // Purple/blue rumble afterglow with some variation
                uint8_t variation = sin8(i * 15 + millis() / 10) / 4;
                uint8_t r = (_rumbleBrightness / 3) + variation / 2;
                uint8_t g = 0;
                uint8_t b = _rumbleBrightness + variation;
                leds[i] = CRGB(r, g, b);
            } else {
                // Dark with occasional dim flicker
                uint8_t ambient = random(0, 8);
                leds[i] = CRGB(ambient / 2, 0, ambient);
            }
        }

        FastLED.show();
    }

private:
    uint32_t _lastUpdate = 0;
    uint32_t _nextFlash = 0;
    uint8_t _flashBrightness = 0;
    uint8_t _rumbleBrightness = 0;
    uint8_t _flashCount = 0;
    bool _inFlashSequence = false;
};

REGISTER_ANIMATION(LightningAnimation, "Lightning Storm");
