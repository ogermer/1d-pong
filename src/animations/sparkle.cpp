#include "animation.h"

class SparkleAnimation : public Animation {
public:
    SparkleAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        _lastShootingStar = 0;
        _shootingStarPos = -1;
        _shootingStarHue = 0;
        // Initialize all stars as off
        for (int i = 0; i < NUM_LEDS; i++) {
            _brightness[i] = 0;
            _targetBrightness[i] = 0;
            _hue[i] = random(256);
        }
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 25) return;
        _lastUpdate = millis();

        // Randomly create new twinkles
        if (random(100) < 15) {
            int pos = random(numLeds);
            if (_targetBrightness[pos] == 0) {
                _targetBrightness[pos] = random(100, 255);
                _hue[pos] = random(256);
            }
        }

        // Update each star
        for (int i = 0; i < numLeds; i++) {
            if (_brightness[i] < _targetBrightness[i]) {
                // Fade in
                _brightness[i] = qadd8(_brightness[i], 20);
                if (_brightness[i] > _targetBrightness[i]) {
                    _brightness[i] = _targetBrightness[i];
                }
            } else if (_brightness[i] > _targetBrightness[i]) {
                // Fade out
                _brightness[i] = qsub8(_brightness[i], 8);
            }

            // Once fully lit, start fading out
            if (_brightness[i] >= _targetBrightness[i] && _targetBrightness[i] > 0) {
                _targetBrightness[i] = 0;
            }

            // Set LED color with pastel saturation
            if (_brightness[i] > 0) {
                leds[i] = CHSV(_hue[i], 180, _brightness[i]);
            } else {
                leds[i] = CRGB::Black;
            }
        }

        // Occasional shooting star
        if (_shootingStarPos < 0 && millis() - _lastShootingStar > 3000 && random(100) < 5) {
            _shootingStarPos = random(2) == 0 ? 0 : numLeds - 1;
            _shootingStarDir = _shootingStarPos == 0 ? 1 : -1;
            _shootingStarHue = random(256);
            _lastShootingStar = millis();
        }

        // Animate shooting star
        if (_shootingStarPos >= 0 && _shootingStarPos < numLeds) {
            // Draw shooting star with trail
            for (int t = 0; t < 6; t++) {
                int p = _shootingStarPos - t * _shootingStarDir;
                if (p >= 0 && p < numLeds) {
                    uint8_t brightness = 255 - t * 45;
                    leds[p] = CHSV(_shootingStarHue, 150, brightness);
                }
            }
            _shootingStarPos += _shootingStarDir * 2;

            // End shooting star when it exits
            if (_shootingStarPos < -5 || _shootingStarPos >= numLeds + 5) {
                _shootingStarPos = -1;
            }
        }

        FastLED.show();
    }

private:
    uint8_t _brightness[NUM_LEDS];
    uint8_t _targetBrightness[NUM_LEDS];
    uint8_t _hue[NUM_LEDS];
    int _shootingStarPos = -1;
    int _shootingStarDir = 1;
    uint8_t _shootingStarHue = 0;
    uint32_t _lastUpdate = 0;
    uint32_t _lastShootingStar = 0;
};

REGISTER_ANIMATION(SparkleAnimation, "Sparkle");
