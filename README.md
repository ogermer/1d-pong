# 1D Pong

A two-player reaction game played on a single strip of WS2812B LEDs. Players compete by pressing their button at the right moment when the ball enters their zone.

## Features

- Fast-paced two-player gameplay
- Dynamic difficulty: zones shrink after each point
- Early-hit bonus: hit the ball as it enters your zone for extra speed
- 13 attract mode animations to draw players in
- Modular animation system for easy customization
- Visual feedback for hits, misses, and scoring
- Configurable game parameters

## Hardware Requirements

### Components

| Component | Specification | Quantity |
|-----------|---------------|----------|
| Microcontroller | WEMOS D1 Mini ESP32 | 1 |
| LED Strip | WS2812B, 55 LEDs | 1 |
| Buttons | Momentary push button | 2 |
| Power Supply | 5V, 3A recommended | 1 |
| Wires | Jumper wires | As needed |

### Wiring Diagram

<!-- Add your wiring diagram image to docs/images/wiring-diagram.png -->
![Wiring Diagram](docs/images/wiring-diagram.png)

### Pin Connections

| Component | ESP32 Pin |
|-----------|-----------|
| LED Data | GPIO 5 |
| Left Button | GPIO 17 |
| Right Button | GPIO 18 |
| LED Power | 5V |
| LED Ground | GND |
| Buttons | GND (active LOW with internal pull-up) |

### Photos

<!-- Add your project photos to docs/images/ -->
![Project Photo](docs/images/project-photo-1.jpg)
![Project Detail](docs/images/project-photo-2.jpg)

## Installation

### Prerequisites

- [VS Code](https://code.visualstudio.com/)
- [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)

### Setup

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/1d-pong.git
   cd 1d-pong
   ```

2. Open the project in VS Code:
   ```bash
   code .
   ```

3. Build the project:
   - Click the checkmark icon in the PlatformIO toolbar, or
   - Press `Ctrl+Alt+B`

4. Upload to your ESP32:
   - Connect your ESP32 via USB
   - Click the arrow icon in the PlatformIO toolbar, or
   - Press `Ctrl+Alt+U`

### Configuration

Edit `include/config.h` to customize:

```cpp
// Hardware
#define NUM_LEDS            55      // Number of LEDs in your strip
#define LED_PIN             5       // Data pin for LED strip
#define BUTTON_LEFT_PIN     17      // Left player button
#define BUTTON_RIGHT_PIN    18      // Right player button

// Game Parameters
#define ZONE_SIZE_START     10      // Initial zone size (LEDs)
#define ZONE_SIZE_MIN       5       // Minimum zone size
#define SCORE_TO_WIN        5       // Points to win a match

// Speed
#define BALL_INITIAL_DELAY_MS  60   // Starting ball speed (lower = faster)

// Animation
#define ANIMATION_DURATION_MS  10000UL  // Duration per animation (ms)
```

## How to Play

### Starting a Game

1. When idle, the LED strip displays attract mode animations
2. Either player presses their button to start a match
3. The ball spawns in the center after a countdown

### Gameplay

1. The ball (white LED) travels toward one player's zone
2. When the ball enters your zone (colored section), press your button to return it
3. The ball speeds up with each successful return
4. Score a point when your opponent misses

### Scoring

- **Miss**: Ball exits the strip = opponent scores
- **Early press**: Pressing outside your zone = opponent scores
- **First to 5 points wins** the match

### Advanced Mechanics

- **Early-hit bonus**: Hitting the ball as it enters your zone (not waiting until it's about to exit) adds extra speed, making it harder for your opponent
- **Shrinking zones**: After each point, both zones shrink by 1 LED, increasing difficulty

## Game State Machine

```
┌───────────────────────────────────────────────────────────────┐
│                                                               │
│  ┌──────────┐    Button     ┌──────────┐                      │
│  │          │    Press      │          │                      │
│  │   IDLE   │──────────────►│  SERVE   │                      │
│  │          │               │          │                      │
│  └──────────┘               └────┬─────┘                      │
│       ▲                          │                            │
│       │                          │ Countdown                  │
│       │                          ▼                            │
│       │                    ┌───────────┐                      │
│       │                    │           │                      │
│       │            ┌──────►│   BALL    │◄─────┐               │
│       │            │       │  MOVING   │      │               │
│       │            │       │           │      │               │
│       │            │       └─────┬─────┘      │               │
│       │            │             │            │               │
│       │            │             │ Miss or    │               │
│       │            │             │ Hit        │               │
│       │            │             ▼            │               │
│       │            │       ┌───────────┐      │               │
│       │            │       │   CHECK   │      │               │
│       │            │       │   GAME    │      │ Not Game      │
│       │            │       │   OVER    │      │ Over          │
│       │            │       └─────┬─────┘      │               │
│       │            │             │            │               │
│       │            │             │ Game       │               │
│       │            │             │ Over?      │               │
│       │            │             ▼            │               │
│       │            │       ┌───────────┐      │               │
│       │            │  No   │           │      │               │
│       │            └───────┤  (check)  ├──────┘               │
│       │                    │           │                      │
│       │                    └─────┬─────┘                      │
│       │                          │ Yes                        │
│       │                          ▼                            │
│       │                    ┌───────────┐                      │
│       │                    │   GAME    │                      │
│       └────────────────────┤   OVER    │                      │
│         Win Animation      │           │                      │
│                            └───────────┘                      │
│                                                               │
└───────────────────────────────────────────────────────────────┘
```

### States

| State | Description |
|-------|-------------|
| `IDLE` | Attract mode - cycling through animations, waiting for player |
| `SERVE` | Countdown before ball launch, zones displayed |
| `BALL_MOVING` | Active gameplay - ball moving, checking for hits |
| `CHECK_GAME_OVER` | Evaluate if a player has won |
| `GAME_OVER` | Display winner animation, then return to idle |

## Adding Custom Animations

The animation system uses auto-registration - simply create a new `.cpp` file in `src/animations/` and it will be automatically included.

### Step-by-Step

1. Copy the template:
   ```bash
   cp src/animations/_template.cpp.example src/animations/my_animation.cpp
   ```

2. Edit your new file:
   ```cpp
   #include "animation.h"

   class MyAnimation : public Animation {
   public:
       MyAnimation(const char* name) : Animation(name) {}

       void reset() override {
           // Initialize state when animation starts
           _lastUpdate = 0;
       }

       void update(CRGB* leds, uint8_t numLeds) override {
           // Non-blocking timing
           if (millis() - _lastUpdate < 50) return;
           _lastUpdate = millis();

           // Your animation logic here
           fill_solid(leds, numLeds, CRGB::Black);
           // ...

           FastLED.show();
       }

   private:
       uint32_t _lastUpdate = 0;
   };

   // Register with a display name
   REGISTER_ANIMATION(MyAnimation, "My Animation");
   ```

3. Build and upload - your animation is now in the rotation!

### Animation Guidelines

- **Non-blocking**: Never use `delay()` - use `millis()` for timing
- **Call `FastLED.show()`**: Required to display your changes
- **Use `reset()`**: Initialize state variables when animation starts
- **Available helpers**: `fill_solid()`, `CHSV()`, `sin8()`, `qadd8()`, `qsub8()`, etc.

### Included Animations

| Animation | Description |
|-----------|-------------|
| Rainbow Dot | Rainbow background with bouncing white dot |
| Pong Demo | Simulated game demonstration |
| Plasma Comet | Plasma background with white comet trail |
| Cylon | Scanning red eye effect |
| Fire | Flames from both player zones |
| Sparkle | Twinkling stars with shooting stars |
| Duel Chase | Blue/green dots colliding |
| Heartbeat | Pulsing red heartbeat |
| Ocean Wave | Blue sine waves with foam |
| Bouncing Balls | Physics-based bouncing balls |
| Matrix Rain | Green digital rain |
| Lightning Storm | Random flashes with thunder |
| Color Breathing | Slow color pulsing |

## Troubleshooting

### LEDs not lighting up

- Check power supply voltage (5V) and amperage (3A recommended for 55 LEDs)
- Verify data pin connection (GPIO 5 by default)
- Ensure correct LED type in config (`WS2812B`)
- Check color order setting (`GRB` for most WS2812B strips)

### Buttons not responding

- Verify button connections to correct GPIO pins
- Buttons should connect between GPIO and GND (uses internal pull-up)
- Check for loose connections

### Upload fails

- Try holding BOOT button on ESP32 while uploading
- Check USB cable (some are power-only)
- Verify correct board selected in `platformio.ini`

### Game too fast/slow

- Adjust `BALL_INITIAL_DELAY_MS` in `config.h`
- Higher values = slower ball
- Recommended range: 40-80ms

## Project Structure

```
1d-pong/
├── platformio.ini          # PlatformIO configuration
├── include/
│   ├── config.h            # Game and hardware configuration
│   └── animation.h         # Animation base class
├── src/
│   ├── main.cpp            # Game logic and state machine
│   ├── animation.cpp       # Animation manager
│   └── animations/         # Animation implementations
│       ├── _template.cpp.example
│       ├── rainbow_dot.cpp
│       ├── fire.cpp
│       └── ...
└── README.md
```

## Contributing

Contributions are welcome! Here's how you can help:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-animation`)
3. **Commit** your changes (`git commit -m 'Add amazing animation'`)
4. **Push** to the branch (`git push origin feature/amazing-animation`)
5. **Open** a Pull Request

### Ideas for Contributions

- New attract mode animations
- Sound effects support (buzzer/speaker)
- Web-based configuration interface
- Score persistence (EEPROM)
- Tournament mode (best of N matches)
- Single-player mode with AI

## Future Improvements

- [ ] Add sound effects for hits and misses
- [ ] Web interface for configuration
- [ ] Bluetooth controller support
- [ ] High score tracking
- [ ] Adjustable difficulty modes
- [ ] More visual feedback options
- [ ] Button LED integration for gameplay feedback and attract animations
  - Note: Button LEDs have integrated resistors for 5V operation and require level shifters or transistors/MOSFETs to drive from ESP32

## Credits

This project was inspired by a 1D Pong game seen at [38C3](https://events.ccc.de/) (38th Chaos Communication Congress).

Built with:
- [FastLED](https://github.com/FastLED/FastLED) - LED control library
- [PlatformIO](https://platformio.org/) - Development platform
- [Arduino-ESP32](https://github.com/espressif/arduino-esp32) - ESP32 Arduino core

## License

This project is licensed under the MIT License - see below for details.

```
MIT License

Copyright (c) 2025

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
