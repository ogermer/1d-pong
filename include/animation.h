#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

// Maximum number of animations that can be registered
#define MAX_ANIMATIONS 16

// ======================================================
// Animation Base Class
// ======================================================
class Animation {
public:
    Animation(const char* name);
    virtual ~Animation() = default;

    // Called once when animation starts
    virtual void reset() {}

    // Called repeatedly to update the animation
    // Should return quickly (non-blocking)
    virtual void update(CRGB* leds, uint8_t numLeds) = 0;

    const char* getName() const { return _name; }

private:
    const char* _name;
};

// ======================================================
// Animation Manager (Singleton)
// ======================================================
class AnimationManager {
public:
    static AnimationManager& getInstance();

    // Register an animation (called automatically by Animation constructor)
    void registerAnimation(Animation* animation);

    // Get animation count
    uint8_t getCount() const { return _count; }

    // Get animation by index
    Animation* getAnimation(uint8_t index) const;

    // Run the current animation, auto-switch after duration
    void update(CRGB* leds, uint8_t numLeds);

    // Force switch to next animation
    void next();

    // Reset to first animation
    void resetToFirst();

    // Check if any button was pressed (to exit attract mode)
    bool wasInterrupted() const { return _interrupted; }
    void clearInterrupt() { _interrupted = false; }
    void interrupt() { _interrupted = true; }

private:
    AnimationManager() : _count(0), _currentIndex(0), _startTime(0), _interrupted(false) {}

    Animation* _animations[MAX_ANIMATIONS];
    uint8_t _count;
    uint8_t _currentIndex;
    uint32_t _startTime;
    bool _interrupted;
};

// ======================================================
// Macro for easy animation registration
// ======================================================
// Usage: REGISTER_ANIMATION(MyAnimation, "My Animation Name")
#define REGISTER_ANIMATION(ClassName, Name) \
    static ClassName _instance_##ClassName(Name)
