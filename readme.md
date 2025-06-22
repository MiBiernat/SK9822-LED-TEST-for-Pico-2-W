# Raspberry Pi Pico SK9822 LED Strip Controller

This project is a C-based firmware for the Raspberry Pi Pico (specifically configured for Pico W) to drive a strip of SK9822 (APA102-compatible) addressable LEDs. It utilizes the Pico's Programmable I/O (PIO) and Direct Memory Access (DMA) for efficient, non-blocking control of the LED strip, minimizing CPU intervention and enabling smooth animations.

The code is designed to manage a strip of 144 LEDs, providing several test patterns and colorful animations.

## Features

* **PIO-based Driver:** A custom PIO program (`sk9822.pio`) generates the precise clock and data signals required by the SK9822 protocol.
* **Efficient DMA Transfer:** DMA is used to send the frame buffer to the PIO state machine, freeing up the CPU for other tasks and ensuring flicker-free animations.
* **Frame Buffer Management:** The code correctly structures the data with start frames (32 bits of zero) and end frames (32 bits of one) as required by the SK9822 specification.
* **Configurable Parameters:** Easily change the number of LEDs, GPIO pins, clock frequency, and global brightness in `led_test_c.c`.
* **Animation and Test Patterns:**
    * A sequential lighting test to verify connectivity.
    * A static color gradient.
    * A yellow and cyan stripes pattern.
    * A smooth, animated rainbow effect.
* **Serial Debug Output:** The program prints its status and the currently running pattern to the serial monitor for easy debugging.

## Hardware Requirements

* **Board:** Raspberry Pi Pico W
* **LEDs:** SK9822 or APA102 LED Strip (configured for 144 LEDs).
* **Wiring:**
    * LED Data Input -> Pico GPIO 11
    * LED Clock Input -> Pico GPIO 10
* An appropriate external power supply for the LED strip.

## Configuration

All main configuration parameters are located at the top of `led_test_c.c`:

```c
// --- Konfiguracja Diód ---
#define NUM_LEDS 144        // Number of LEDs in the strip
#define DATA_PIN 11         // Data pin
#define CLOCK_PIN 10        // Clock pin

// Recommended safe clock speed for SK9822/APA102 is 15-25 MHz
#define LED_CLOCK_FREQ (15 * 1000 * 1000)

// Default global brightness (0-31)
#define GLOBAL_BRIGHTNESS_VALUE 10