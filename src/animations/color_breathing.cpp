#include "animation.h"

class ColorBreathingAnimation : public Animation {
public:
    ColorBreathingAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        _phase = 0;
        _currentHue = 0;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 20) return;
        _lastUpdate = millis();

        _phase++;

        // Slow breathing cycle (~4 seconds per breath)
        // Using sine wave for smooth organic breathing
        uint8_t breathPhase = _phase;  // 0-255 over ~5 seconds

        // sin8 gives 0-255, we want breathing to go 0 -> max -> 0
        uint8_t rawBreath = sin8(breathPhase);

        // Apply gamma correction for more natural breathing feel
        // Spend more time at low brightness, quick rise to peak
        uint8_t brightness = ease8InOutQuad(rawBreath);

        // Scale to avoid being too dim at bottom
        brightness = 20 + (brightness * 235 / 255);

        // Change color at the bottom of each breath (when dimmest)
        if (breathPhase < 3 && _lastBreathPhase > 250) {
            _currentHue += 32;  // Shift hue for next breath
        }
        _lastBreathPhase = breathPhase;

        // Fill with current color and brightness
        // Add subtle position-based hue variation for depth
        for (int i = 0; i < numLeds; i++) {
            uint8_t hueOffset = (i * 3) - (numLeds / 2);
            leds[i] = CHSV(_currentHue + hueOffset, 220, brightness);
        }

        FastLED.show();
    }

private:
    uint32_t _lastUpdate = 0;
    uint8_t _phase = 0;
    uint8_t _currentHue = 0;
    uint8_t _lastBreathPhase = 0;
};

REGISTER_ANIMATION(ColorBreathingAnimation, "Color Breathing");
