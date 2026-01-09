#include "animation.h"

class BouncingBallsAnimation : public Animation {
public:
    BouncingBallsAnimation(const char* name) : Animation(name) {}

    void reset() override {
        _lastUpdate = 0;

        // Initialize balls at different starting positions
        for (int i = 0; i < NUM_BALLS; i++) {
            _ballPos[i] = random(5, NUM_LEDS - 5);
            _ballVel[i] = 0;
            _ballGround[i] = (i % 2 == 0) ? 0 : NUM_LEDS - 1;  // Alternate ground sides
            _ballColor[i] = CHSV(i * 60 + 20, 255, 255);  // Different colors
            _timeSinceKick[i] = random(0, 3000);  // Stagger initial kicks
        }
    }

    void update(CRGB* leds, uint8_t numLeds) override {
        if (millis() - _lastUpdate < 20) return;
        uint32_t deltaTime = millis() - _lastUpdate;
        _lastUpdate = millis();

        // Clear with fade for trail effect
        for (int i = 0; i < numLeds; i++) {
            leds[i].fadeToBlackBy(80);
        }

        // Update each ball
        for (int b = 0; b < NUM_BALLS; b++) {
            _timeSinceKick[b] += deltaTime;

            // Gravity pulls toward the ball's ground
            float gravity = (_ballGround[b] == 0) ? -0.15f : 0.15f;

            // Apply gravity
            _ballVel[b] += gravity;

            // Apply velocity
            _ballPos[b] += _ballVel[b];

            // Bounce off ground
            if (_ballGround[b] == 0 && _ballPos[b] <= 0) {
                _ballPos[b] = 0;
                _ballVel[b] = -_ballVel[b] * DAMPING;

                // Re-kick if ball has settled
                if (abs(_ballVel[b]) < 0.5f) {
                    _ballVel[b] = 0;
                }
            } else if (_ballGround[b] == numLeds - 1 && _ballPos[b] >= numLeds - 1) {
                _ballPos[b] = numLeds - 1;
                _ballVel[b] = -_ballVel[b] * DAMPING;

                if (abs(_ballVel[b]) < 0.5f) {
                    _ballVel[b] = 0;
                }
            }

            // Periodically kick the ball back up
            if (_timeSinceKick[b] > 2500 + random(0, 1500) && abs(_ballVel[b]) < 1.0f) {
                float kickStrength = 3.5f + random(0, 20) / 10.0f;
                _ballVel[b] = (_ballGround[b] == 0) ? kickStrength : -kickStrength;
                _timeSinceKick[b] = 0;
            }

            // Clamp position
            if (_ballPos[b] < 0) _ballPos[b] = 0;
            if (_ballPos[b] >= numLeds) _ballPos[b] = numLeds - 1;

            // Draw ball with small glow
            int pos = (int)_ballPos[b];
            leds[pos] = _ballColor[b];

            // Subtle glow around ball
            if (pos > 0) leds[pos - 1] += _ballColor[b] % 64;
            if (pos < numLeds - 1) leds[pos + 1] += _ballColor[b] % 64;
        }

        FastLED.show();
    }

private:
    static const int NUM_BALLS = 4;
    static constexpr float DAMPING = 0.75f;

    float _ballPos[NUM_BALLS];
    float _ballVel[NUM_BALLS];
    int _ballGround[NUM_BALLS];
    CRGB _ballColor[NUM_BALLS];
    uint32_t _timeSinceKick[NUM_BALLS];
    uint32_t _lastUpdate = 0;
};

REGISTER_ANIMATION(BouncingBallsAnimation, "Bouncing Balls");
