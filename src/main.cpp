#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "animation.h"

// ======================================================
// LED Array
// ======================================================
CRGB leds[NUM_LEDS];

// ======================================================
// Game State
// ======================================================
enum PlayerSide : uint8_t { PLAYER_LEFT = 0, PLAYER_RIGHT = 1 };

struct ButtonEvent {
    PlayerSide player;
    uint32_t   timestampMs;
};

QueueHandle_t buttonQueue;

enum GameState {
    STATE_IDLE,
    STATE_SERVE,
    STATE_BALL_MOVING,
    STATE_CHECK_GAME_OVER,
    STATE_GAME_OVER
};

volatile GameState currentState = STATE_IDLE;

int      ballPos        = NUM_LEDS / 2;
int      ballDir        = 1;
uint16_t ballDelayMs    = BALL_DELAY_START;
uint8_t  scoreLeft      = 0;
uint8_t  scoreRight     = 0;
uint8_t  currentZoneSize = ZONE_SIZE_START;
PlayerSide lastLoser    = PLAYER_LEFT;

// ======================================================
// Rendering Helpers
// ======================================================
void clearLeds() {
    fill_solid(leds, NUM_LEDS, COLOR_BACKGROUND);
}

void drawZones() {
    for (uint8_t i = 0; i < currentZoneSize; i++) {
        leds[i] = COLOR_ZONE_LEFT;
        leds[NUM_LEDS - 1 - i] = COLOR_ZONE_RIGHT;
    }
}

void drawBall() {
    if (ballPos >= 0 && ballPos < NUM_LEDS) {
        leds[ballPos] = COLOR_BALL;

        // 3-LED comet trail
        int t1 = ballPos - ballDir;
        int t2 = ballPos - 2 * ballDir;
        int t3 = ballPos - 3 * ballDir;

        if (t1 >= 0 && t1 < NUM_LEDS) {
            leds[t1] = CRGB::White;
            leds[t1].fadeToBlackBy(160);
        }
        if (t2 >= 0 && t2 < NUM_LEDS) {
            leds[t2] = CRGB::White;
            leds[t2].fadeToBlackBy(210);
        }
        if (t3 >= 0 && t3 < NUM_LEDS) {
            leds[t3] = CRGB::White;
            leds[t3].fadeToBlackBy(240);
        }
    }
}

void showKeypressFeedback(PlayerSide player) {
    CRGB savedLeft[ZONE_SIZE_START];
    CRGB savedRight[ZONE_SIZE_START];

    for (uint8_t i = 0; i < currentZoneSize; i++) {
        savedLeft[i]  = leds[i];
        savedRight[i] = leds[NUM_LEDS - 1 - i];
    }

    CRGB orange = CRGB(255, 80, 0);
    if (player == PLAYER_LEFT) {
        for (uint8_t i = 0; i < currentZoneSize; i++) leds[i] = orange;
    } else {
        for (uint8_t i = 0; i < currentZoneSize; i++) leds[NUM_LEDS - 1 - i] = orange;
    }

    FastLED.show();
    vTaskDelay(pdMS_TO_TICKS(80));

    if (player == PLAYER_LEFT) {
        for (uint8_t i = 0; i < currentZoneSize; i++) leds[i] = savedLeft[i];
    } else {
        for (uint8_t i = 0; i < currentZoneSize; i++) leds[NUM_LEDS - 1 - i] = savedRight[i];
    }

    FastLED.show();
}

void drawScoreOverlay() {
    const uint8_t c = NUM_LEDS / 2;
    for (uint8_t i = 0; i < scoreLeft && (c - 1 - i) >= 0; i++) {
        leds[c - 1 - i] += CRGB(0, 0, 100);
    }
    for (uint8_t i = 0; i < scoreRight && (c + 1 + i) < NUM_LEDS; i++) {
        leds[c + 1 + i] += CRGB(0, 100, 0);
    }
}

void showMissAnimation(PlayerSide p) {
    for (uint8_t f = 0; f < 3; f++) {
        clearLeds();
        if (p == PLAYER_LEFT) {
            for (uint8_t i = 0; i < currentZoneSize; i++) leds[i] = COLOR_MISS;
        } else {
            for (uint8_t i = 0; i < currentZoneSize; i++) leds[NUM_LEDS - 1 - i] = COLOR_MISS;
        }
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(120));
        clearLeds();
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

void showWinAnimation(PlayerSide w) {
    CRGB col = (w == PLAYER_LEFT) ? COLOR_WIN_LEFT : COLOR_WIN_RIGHT;
    for (uint8_t r = 0; r < 10; r++) {
        fill_solid(leds, NUM_LEDS, col);
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(120));
        clearLeds();
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(80));
    }
    currentZoneSize = ZONE_SIZE_START;
}

void resetMatch() {
    scoreLeft = scoreRight = 0;
    ballDelayMs = BALL_DELAY_START;
    lastLoser = PLAYER_LEFT;
    currentZoneSize = ZONE_SIZE_START;
}

int randomDirection() {
    return (random(0, 2) == 0) ? -1 : 1;
}

void prepareServe() {
    ballPos = NUM_LEDS / 2;
    ballDelayMs = BALL_DELAY_START;

    if (scoreLeft == 0 && scoreRight == 0) {
        ballDir = randomDirection();
    } else {
        ballDir = (lastLoser == PLAYER_LEFT) ? -1 : +1;
    }

    if (scoreLeft + scoreRight > 0 && currentZoneSize > ZONE_SIZE_MIN) {
        currentZoneSize--;
    }

    for (int c = 3; c > 0; --c) {
        clearLeds();
        drawZones();
        leds[NUM_LEDS / 2] = CRGB::Yellow;
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(200));
        clearLeds();
        drawZones();
        FastLED.show();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// ======================================================
// Tasks
// ======================================================
void buttonTask(void* pvParameters) {
    (void)pvParameters;
    pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);

    bool lastL = digitalRead(BUTTON_LEFT_PIN) == BUTTON_ACTIVE_LEVEL;
    bool lastR = digitalRead(BUTTON_RIGHT_PIN) == BUTTON_ACTIVE_LEVEL;
    bool stableL = lastL, stableR = lastR;
    uint32_t changeL = millis(), changeR = millis();
    const uint32_t debounceMs = 20;

    for (;;) {
        uint32_t now = millis();
        bool rawL = digitalRead(BUTTON_LEFT_PIN) == BUTTON_ACTIVE_LEVEL;
        bool rawR = digitalRead(BUTTON_RIGHT_PIN) == BUTTON_ACTIVE_LEVEL;

        if (rawL != lastL) {
            lastL = rawL;
            changeL = now;
        } else if ((now - changeL) >= debounceMs && stableL != rawL) {
            stableL = rawL;
            if (stableL) {
                ButtonEvent ev{PLAYER_LEFT, now};
                xQueueSend(buttonQueue, &ev, 0);
            }
        }

        if (rawR != lastR) {
            lastR = rawR;
            changeR = now;
        } else if ((now - changeR) >= debounceMs && stableR != rawR) {
            stableR = rawR;
            if (stableR) {
                ButtonEvent ev{PLAYER_RIGHT, now};
                xQueueSend(buttonQueue, &ev, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void gameTask(void* pvParameters) {
    (void)pvParameters;
    currentState = STATE_IDLE;

    AnimationManager& animManager = AnimationManager::getInstance();
    animManager.resetToFirst();

    Serial.printf("Loaded %d animations\n", animManager.getCount());

    for (;;) {
        switch (currentState) {

        case STATE_IDLE: {
            ButtonEvent ev;
            if (xQueueReceive(buttonQueue, &ev, 0) == pdTRUE) {
                resetMatch();
                clearLeds();
                FastLED.show();
                currentState = STATE_SERVE;
                break;
            }
            // Run attract mode animations
            animManager.update(leds, NUM_LEDS);
            break;
        }

        case STATE_SERVE:
            prepareServe();
            currentState = STATE_BALL_MOVING;
            break;

        case STATE_BALL_MOVING: {
            bool leftPressed = false, rightPressed = false;
            ButtonEvent ev;
            while (xQueueReceive(buttonQueue, &ev, 0) == pdTRUE) {
                if (ev.player == PLAYER_LEFT) leftPressed = true;
                if (ev.player == PLAYER_RIGHT) rightPressed = true;
            }

            ballPos += ballDir;

            bool inLeft  = (ballPos >= 0 && ballPos < (int)currentZoneSize);
            bool inRight = (ballPos >= (int)NUM_LEDS - (int)currentZoneSize);

            // Penalty: press outside zone
            if (leftPressed && !inLeft) {
                showMissAnimation(PLAYER_LEFT);
                scoreRight++;
                lastLoser = PLAYER_LEFT;
                currentState = STATE_CHECK_GAME_OVER;
                break;
            }
            if (rightPressed && !inRight) {
                showMissAnimation(PLAYER_RIGHT);
                scoreLeft++;
                lastLoser = PLAYER_RIGHT;
                currentState = STATE_CHECK_GAME_OVER;
                break;
            }

            // Successful hit with early-hit bonus
            // Early hit = ball just entered zone, late hit = ball about to exit
            if (inLeft && leftPressed) {
                showKeypressFeedback(PLAYER_LEFT);
                ballDir = +1;

                // Calculate early hit factor: 1.0 at zone entry, 0.0 at zone exit
                // Left zone: entry at currentZoneSize-1, exit at 0
                float earlyFactor = (currentZoneSize > 1)
                    ? (float)ballPos / (float)(currentZoneSize - 1)
                    : 0.0f;
                uint16_t totalSpeedup = BALL_SPEEDUP_PER_RETURN +
                    (uint16_t)(earlyFactor * BALL_EARLY_HIT_MAX_BONUS);

                if (ballDelayMs > BALL_DELAY_MIN + totalSpeedup)
                    ballDelayMs -= totalSpeedup;
                else
                    ballDelayMs = BALL_DELAY_MIN;
            }
            if (inRight && rightPressed) {
                showKeypressFeedback(PLAYER_RIGHT);
                ballDir = -1;

                // Calculate early hit factor: 1.0 at zone entry, 0.0 at zone exit
                // Right zone: entry at NUM_LEDS-currentZoneSize, exit at NUM_LEDS-1
                float earlyFactor = (currentZoneSize > 1)
                    ? (float)(NUM_LEDS - 1 - ballPos) / (float)(currentZoneSize - 1)
                    : 0.0f;
                uint16_t totalSpeedup = BALL_SPEEDUP_PER_RETURN +
                    (uint16_t)(earlyFactor * BALL_EARLY_HIT_MAX_BONUS);

                if (ballDelayMs > BALL_DELAY_MIN + totalSpeedup)
                    ballDelayMs -= totalSpeedup;
                else
                    ballDelayMs = BALL_DELAY_MIN;
            }

            // Normal miss
            if (ballPos < 0) {
                showMissAnimation(PLAYER_LEFT);
                scoreRight++;
                lastLoser = PLAYER_LEFT;
                currentState = STATE_CHECK_GAME_OVER;
                break;
            }
            if (ballPos >= NUM_LEDS) {
                showMissAnimation(PLAYER_RIGHT);
                scoreLeft++;
                lastLoser = PLAYER_RIGHT;
                currentState = STATE_CHECK_GAME_OVER;
                break;
            }

            // Render frame
            clearLeds();
            drawZones();
            drawBall();
            drawScoreOverlay();
            FastLED.show();

            vTaskDelay(pdMS_TO_TICKS(ballDelayMs));
            break;
        }

        case STATE_CHECK_GAME_OVER:
            currentState = (scoreLeft >= SCORE_TO_WIN || scoreRight >= SCORE_TO_WIN)
                           ? STATE_GAME_OVER : STATE_SERVE;
            break;

        case STATE_GAME_OVER: {
            PlayerSide winner = (scoreLeft >= SCORE_TO_WIN) ? PLAYER_LEFT : PLAYER_RIGHT;
            showWinAnimation(winner);
            // Reset to first animation after game ends
            animManager.resetToFirst();
            currentState = STATE_IDLE;
            break;
        }
        }
    }
}

// ======================================================
// Setup & Loop
// ======================================================
void setup() {
    Serial.begin(115200);
    delay(200);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    clearLeds();
    FastLED.show();

    buttonQueue = xQueueCreate(10, sizeof(ButtonEvent));

    xTaskCreatePinnedToCore(buttonTask, "Btn",  4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(gameTask,   "Game", 8192, NULL, 1, NULL, 1);

    Serial.println("1D-Pong - Modular Animation System");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
