# BUSTEG OS Code Tour

This app is a small C program. It is meant to feel like a tiny custom operating
system, but it still runs as one normal Linux program called `cyberdeck-os`.

## Start Here

Read these files in this order:

1. `src/main.c`
   - Creates one `App`.
   - Calls `App_Init`, `App_Run`, and `App_Shutdown`.
2. `src/core/app.c`
   - Wires together display, input, events, screens, and modules.
   - Contains the main loop: read input, update state, draw one frame.
3. `src/core/screen_manager.c`
   - Decides which screen is visible: boot, splash, home, or a module.
4. `src/screens/home.c`
   - Shows the menu and opens modules.
5. `src/modules/`
   - Each file here is one tool on the home screen.

## The Main Loop

Every frame does the same simple steps:

```text
read keyboard/mouse/touch input
turn input into NavEvent values
send events to the current screen
update modules
draw the current screen
sleep a tiny bit
```

The app does not let modules read SDL events directly. Modules receive simple
navigation events like `NAV_UP`, `NAV_SELECT`, and `NAV_BACK`.

## Adding a New Module

Copy one of the simple modules, such as `src/modules/module_settings.c`.

Then:

1. Create `module_mytool.c` and `module_mytool.h`.
2. Add `#include "modules/module_mytool.h"` to `src/core/app.c`.
3. Add your module in `register_modules()` in `src/core/app.c`.
4. Rebuild.

A module can provide these callbacks:

```text
init         run once when the app starts
update       run every frame
render       draw the module screen
handle_event handle buttons/touch
shutdown     clean up before exit
```

Only `render` is truly needed for a simple read-only screen.

## Important Folders

```text
src/core/      app wiring, screen state, event queue, config
src/hal/       hardware-ish layer: display, input, serial, system info
src/screens/   boot/splash/home screens
src/modules/   tools shown on the home menu
src/ui/        small drawing helpers
tools/         helper programs used in the Pi image
```

## Safe Things To Experiment With

- Change text in `src/screens/splash.c`.
- Change menu modules in `register_modules()` inside `src/core/app.c`.
- Change colors in `config.ini`.
- Add labels to `src/modules/module_settings.c`.
- Tweak the Snake board speed/size constants in `src/modules/module_snake.c`.
- Make a new read-only module that draws text with `Display_DrawText`.

## Files To Treat Carefully

- `src/hal/display.c`
  - Talks to SDL and the Raspberry Pi renderer. Small changes can make the
    screen fail to start.
- `src/hal/input.c`
  - Has both SDL input and Linux `/dev/input/event*` keyboard fallback.
- `tools/fb_splash.c`
  - Draws the logo before the main app starts.

## Rebuild For Raspberry Pi

For boot, input, or display changes, rebuild the boot-sensitive packages:

```sh
make pi-boot-image
```

For simple app-only experiments on a Linux desktop with SDL installed:

```sh
make -C cyberdeck-os/app
```
