#include "animation.h"

// ======================================================
// Animation Base Class Implementation
// ======================================================
Animation::Animation(const char* name) : _name(name) {
    AnimationManager::getInstance().registerAnimation(this);
}

// ======================================================
// Animation Manager Implementation
// ======================================================
AnimationManager& AnimationManager::getInstance() {
    static AnimationManager instance;
    return instance;
}

void AnimationManager::registerAnimation(Animation* animation) {
    if (_count < MAX_ANIMATIONS) {
        _animations[_count++] = animation;
    }
}

Animation* AnimationManager::getAnimation(uint8_t index) const {
    if (index < _count) {
        return _animations[index];
    }
    return nullptr;
}

void AnimationManager::update(CRGB* leds, uint8_t numLeds) {
    if (_count == 0) return;

    // Initialize start time on first call
    if (_startTime == 0) {
        _startTime = millis();
        _animations[_currentIndex]->reset();
    }

    // Check if it's time to switch animations
    if (millis() - _startTime >= ANIMATION_DURATION_MS) {
        next();
    }

    // Update current animation
    _animations[_currentIndex]->update(leds, numLeds);
}

void AnimationManager::next() {
    _currentIndex = (_currentIndex + 1) % _count;
    _startTime = millis();
    _animations[_currentIndex]->reset();
}

void AnimationManager::resetToFirst() {
    _currentIndex = 0;
    _startTime = millis();
    if (_count > 0) {
        _animations[_currentIndex]->reset();
    }
}
