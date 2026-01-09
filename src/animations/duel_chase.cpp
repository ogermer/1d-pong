#include "animation.h"

class DuelChaseAnimation : public Animation {
public:
    DuelChaseAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;
        _leftPos = 0;
        _rightPos = NUM_LEDS - 1;
        _leftSpeed = 1.0f;
        _rightSpeed = 1.0f;
        _collisionFlash = 0;
        _phase = PHASE_APPROACH;
        _pauseUntil = 0;
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 35) return;
        _lastUpdate = millis();

        // Clear strip
        fill_solid(leds, numLeds, CRGB::Black);

        // Handle collision flash
        if (_collisionFlash > 0) {
            uint8_t brightness = _collisionFlash;
            fill_solid(leds, numLeds, CRGB(brightness, brightness, brightness));
            _collisionFlash = qsub8(_collisionFlash, 40);
            FastLED.show();
            return;
        }

        // Handle pause
        if (millis() < _pauseUntil) {
            drawDots(leds, numLeds);
            FastLED.show();
            return;
        }

        switch (_phase) {
            case PHASE_APPROACH:
                // Move toward each other, accelerating
                _leftSpeed = min(_leftSpeed + 0.05f, 2.5f);
                _rightSpeed = min(_rightSpeed + 0.05f, 2.5f);
                _leftPos += _leftSpeed;
                _rightPos -= _rightSpeed;

                // Check for collision in the middle
                if (_leftPos >= _rightPos - 2) {
                    _collisionFlash = 255;
                    _phase = PHASE_RETREAT;
                    _leftSpeed = 2.0f;
                    _rightSpeed = 2.0f;
                }
                break;

            case PHASE_RETREAT:
                // Move back to edges, decelerating
                _leftSpeed = max(_leftSpeed - 0.03f, 0.5f);
                _rightSpeed = max(_rightSpeed - 0.03f, 0.5f);
                _leftPos -= _leftSpeed;
                _rightPos += _rightSpeed;

                // Return to starting positions
                if (_leftPos <= 0) {
                    _leftPos = 0;
                    _leftSpeed = 0;
                }
                if (_rightPos >= numLeds - 1) {
                    _rightPos = numLeds - 1;
                    _rightSpeed = 0;
                }

                // Both returned - pause then restart
                if (_leftPos <= 0 && _rightPos >= numLeds - 1) {
                    _pauseUntil = millis() + 800;
                    _phase = PHASE_APPROACH;
                    _leftSpeed = 0.5f;
                    _rightSpeed = 0.5f;
                }
                break;
        }

        drawDots(leds, numLeds);
        FastLED.show();
    }

private:
    enum Phase { PHASE_APPROACH, PHASE_RETREAT };

    float _leftPos = 0;
    float _rightPos = NUM_LEDS - 1;
    float _leftSpeed = 1.0f;
    float _rightSpeed = 1.0f;
    uint8_t _collisionFlash = 0;
    Phase _phase = PHASE_APPROACH;
    uint32_t _lastUpdate = 0;
    uint32_t _pauseUntil = 0;

    void drawDots(CRGB* leds, uint8_t numLeds) {
        // Draw left player dot (blue) with trail
        for (int t = 0; t < 4; t++) {
            int p = (int)_leftPos - t;
            if (p >= 0 && p < numLeds) {
                uint8_t brightness = 255 - t * 60;
                leds[p] = CRGB(0, 0, brightness);
            }
        }

        // Draw right player dot (green) with trail
        for (int t = 0; t < 4; t++) {
            int p = (int)_rightPos + t;
            if (p >= 0 && p < numLeds) {
                uint8_t brightness = 255 - t * 60;
                leds[p] = CRGB(0, brightness, 0);
            }
        }
    }
};

REGISTER_ANIMATION(DuelChaseAnimation, "Duel Chase");
