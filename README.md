# FEUP Heist

FEUP Heist is a 2D stealth game for the LCOM/Minix environment. The player controls a FEUP student who must collect the exam answers from a safe, escape through two levels and reach the car before the timer runs out. Teachers patrol the maze and chase the player on sight.


## 📺 Video Demonstration

[Watch the video demonstration](video/lcom_projeto_video_demo.mp4)

## Build and Run

```sh
make clean
make
lcom_run proj
```

If the program gets stuck:

```sh
lcom_stop proj
```

## Controls

| Input | Action |
|---|---|
| `WASD` or arrow keys | Move the player |
| `P` | Pause / resume |
| `Enter` | Confirm, start or restart |
| `Esc` | Clear drawing canvas |
| `1-4` | Select quiz answer |
| Mouse movement | Move the on-screen cursor |
| Left mouse button | Menu navigation and drawing canvas |

## Gameplay

You are a FEUP student with an incoming LCOM test you haven't studied for. Your plan: steal the exam answers from the teacher's safe and escape before the teaching staff catches you.

- **Level 1 — Indoor:** Navigate the university corridors, answer LCOM quiz questions to open the safe and collect the exam sheet while dodging patrolling teachers.
- **Level 2 — Outdoor:** Escape through the university grounds, reach your car and drive to safety before the timer runs out.

If a teacher spots you, they will chase you. Get caught or run out of time and it's game over.

## Devices Used

| Device | Usage |
|---|---|
| Timer | Game loop, countdown, teacher movement and animation timing |
| Keyboard | Player movement, pause, restart and menu navigation |
| Mouse | Cursor movement, menu clicks and drawing quiz input |
| Graphics card | VBE graphics mode, double buffering, XPM sprites, HUD and menus |
| RTC | Current date and time shown in HUD and associated with high scores |
| Serial port | COM1 event log for game milestones (exam collected, win, lose, etc.) |

## Project Structure

```
projeto/
├── Sprites/
│   ├── PNGs/                       PNG versions of the game sprites
│   └── XPMs/                       XPM sprites used by the game
├── data/
│   └── Questions.txt               Quiz questions used during gameplay
├── include/
│   └── game.h                      Public game interface
├── libs/
│   ├── devices/                    Low-level device modules
│   │   ├── timer.c / .h
│   │   ├── kbc.c / .h
│   │   ├── mouse.c / .h
│   │   ├── rtc.c / .h
│   │   ├── serial.c / .h
│   │   ├── i8042.h
│   │   └── i8254.h
│   ├── graphics/
│   │   └── graphics.c / .h         VBE mode, VRAM mapping, drawing and buffering
│   └── utils/
│       ├── bitwise.c / .h          Bit manipulation helpers
│       └── utils.c / .h            Auxiliary functions
├── src/
│   ├── proj.c                      Main loop, device subscriptions and interrupt dispatch
│   ├── game.c                      Maps, quiz data, global resources and shared game data
│   ├── game_init.c                 Game initialization, level setup and sprite loading
│   ├── game_input.c                Keyboard and mouse input handling
│   ├── game_internal.h             Internal declarations shared by the game modules
│   ├── game_quiz.c                 Multiple-choice quiz logic and drawing recognition
│   ├── game_render.c               Rendering of the game, menus, guide, HUD and quiz screens
│   ├── game_text.c                 Text and digit drawing utilities
│   └── game_update.c               Game rules, teacher behaviour, movement and collisions
├── Doxyfile 						Documentation configuration
├── Makefile                        Compilation rules and module organization
└── README.md
```

## Authors

- Matilde Oliveira Silva — up202305722


