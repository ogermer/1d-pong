#include "animation.h"

class PongDemoAnimation : public Animation {
public:
    PongDemoAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _ballPos = NUM_LEDS / 2;
        _ballDir = 1;
        _delay = 120;
        _lastUpdate = 0;
        _zoneSize = ZONE_SIZE_START;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < _delay) return;
        _lastUpdate = millis();

        // Clear and draw zones
        fill_solid(leds, numLeds, CRGB::Black);
        for (uint8_t i = 0; i < _zoneSize; i++) {
            leds[i] = COLOR_ZONE_LEFT;
            leds[numLeds - 1 - i] = COLOR_ZONE_RIGHT;
        }

        // Bounce at zones
        if ((_ballPos < _zoneSize && _ballDir == -1) ||
            (_ballPos >= numLeds - _zoneSize && _ballDir == 1)) {
            _ballDir = -_ballDir;
            if (_delay > 50) _delay -= 4;
        }

        _ballPos += _ballDir;

        // Reset if ball exits
        if (_ballPos <= 0 || _ballPos >= numLeds - 1) {
            _ballPos = numLeds / 2;
            _delay = 120;
            _ballDir = (random(0, 2) == 0) ? -1 : 1;
        }

        leds[_ballPos] = CRGB::White;
        FastLED.show();
    }

private:
    int _ballPos = NUM_LEDS / 2;
    int _ballDir = 1;
    uint16_t _delay = 120;
    uint32_t _lastUpdate = 0;
    uint8_t _zoneSize = ZONE_SIZE_START;
};

REGISTER_ANIMATION(PongDemoAnimation, "Pong Demo");
