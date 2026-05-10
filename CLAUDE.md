# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Robotic crane/arm controller for the **ATmega2560** (Arduino Mega 2560) built with **PlatformIO** + Arduino framework, but written as bare-metal AVR C — direct register access (`DDRx`, `TCCRxA/B`, `OCRxA`, `ADMUX`, `UCSR0x`, `TWCR`, ISRs). The Arduino framework is essentially only providing toolchain/uploader; do not introduce Arduino API calls (`digitalWrite`, `Serial`, `analogRead`, etc.).

The board drives 4 servos and reads either a 2×2-axis analog joystick (ADC) or keystrokes from a serial terminal (UART). An SSD1306 OLED on I2C shows live joystick values.

## Build / Upload / Monitor

```
pio run                           # build
pio run -t upload                 # build + flash via USB
pio device monitor -b 19200       # serial monitor — UART is 19200, NOT the PIO default
pio run -t clean                  # clean build artifacts
```

The serial UI prompts the user to press `1` (KEYBOARD) or `0` (JOYSTICK). In keyboard mode the controls are `a`/`d` (X1), `w`/`s` (Y1), `i`/`o` (Y2), `k`/`l` (X2); pressing `0` switches back to joystick mode.

There is no test runner wired up — the `test/` directory only contains the default PlatformIO README.

## Architecture

`main.c` is the orchestrator. Initialization order matters: `uart_init` → `init_all_PWM` → `init_adc` → `sei()` → `CTC_init` → `I2C_Init` → `InitializeDisplay` → `clear_display`. Three ISRs drive everything; the main loop is a state machine over `controleState { keyboardState, joystickState }` plus a `choosingState` flag that gates the welcome prompt.

**ISR-driven data flow:**
- `TIMER0_COMPA_vect` (CTC, prescaler 64, OCR0A=249 → ~1 kHz tick) cycles `channel` 0→3 and kicks off the next ADC conversion.
- `ADC_vect` stores the result into `Adcvalues[channel]` and sets `Adcready`.
- `USART0_RX_vect` stores the byte into `bruh` and sets `RX0_COMPLETE_FLAG`.

The main loop polls those flags, never blocks on UART/ADC except at the very first state-selection prompt.

**Servo PWM (Timer1/3/4/5, phase-and-frequency-correct mode, 50 Hz):**
- `ICR=20000`, `CS11` prescaler (8) → 1 tick = 1 µs, so `OCRxA` values are pulse width in µs.
- Valid servo range is **500–2500 µs**; any new code that writes `OCRxA` directly must clamp to this range or go through `setpwm` / `setpwm_UART` which already do.
- Channel-to-timer mapping is fixed via `PWMbuffer[4] = {&OCR1A, &OCR3A, &OCR4A, &OCR5A}` indexed by the `enum cord { X1, Y1, X2, Y2 }`. Pins are PB5, PE3, PH3, PL3 respectively. If you reorder the enum you also reorder the physical axes.
- Both setters move the pulse width by ±10 µs per call (ramping, not snapping) — this is intentional for smooth servo motion. Don't "optimize" it to a direct write.

**ADC scanning:** uses internal AVcc reference (`REFS0`), prescaler 128 (~125 kHz ADC clock), interrupt-complete enabled. `select_channel(ch)` rewrites `ADMUX` each tick — it overwrites the reference bits, so if you change the reference, fix `select_channel` too.

**I2C / SSD1306:** `I2C.c` is a polled TWI driver from AVR Freaks (status-code based). `ssd1306.c` is an Adafruit-derived port using I2C addr 0x78 write / 0x7A read. `F_CPU` (16 MHz) and `SCL_CLK` (100 kHz) are defined inside `include/I2C.h`, not in `platformio.ini` — adding `-D F_CPU=...` build flags will collide.

**UART:** USART0 at 19200 baud, 8N1, RX-complete interrupt enabled. `putstringuart` is blocking on `UDRE0`.

## Things to know before editing

- `clock.c` references globals (`Clock`, `Alarm`, `GlobalRxBuffer`) declared `extern` in `include/clock.h` but **not defined anywhere**. The clock/alarm feature is not currently wired into `main.c`; calling `checkCLOCKstate` or `Alarmreached` from main will fail to link until the externs are defined.
- `include/I2C.h` declares globals (`write_address`, `read_addres`, `_i2c_address` in `ssd1306.h`) at file scope without `extern`. This works because the header is only included in one translation unit each — including either header in a second `.c` file will produce multiple-definition link errors.
- `main.c` has duplicated `#include` lines at the top (kept verbatim from the working firmware — do not "clean up" without re-testing on hardware).
- `enum cord` and the `KEYBOARD`/`JOYSTICK` macros (49, 48 = ASCII `'1'`, `'0'`) are defined inline in `main.c`; they are not in a shared header.
- Source files use `.c` extension and bare-metal AVR headers (`<avr/io.h>`, `<avr/interrupt.h>`, `<util/delay.h>`). Don't switch to C++ unless you also rename and add `extern "C"` shims for the existing modules.
