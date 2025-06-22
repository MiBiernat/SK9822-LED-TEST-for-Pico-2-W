# Raspberry Pi Pico SK9822 LED Strip Controller

[cite_start]This project is a C-based firmware for the Raspberry Pi Pico (specifically configured for Pico W ) to drive a strip of SK9822 (APA102-compatible) addressable LEDs. [cite_start]It utilizes the Pico's Programmable I/O (PIO) and Direct Memory Access (DMA) for efficient, non-blocking control of the LED strip, minimizing CPU intervention and enabling smooth animations. 

[cite_start]The code is designed to manage a strip of 144 LEDs, providing several test patterns and colorful animations. 

## Features

* [cite_start]**PIO-based Driver:** A custom PIO program (`sk9822.pio`) generates the precise clock and data signals required by the SK9822 protocol. 
* [cite_start]**Efficient DMA Transfer:** DMA is used to send the frame buffer to the PIO state machine, freeing up the CPU for other tasks and ensuring flicker-free animations. 
* [cite_start]**Frame Buffer Management:** The code correctly structures the data with start frames (32 bits of zero) and end frames (32 bits of one) as required by the SK9822 specification. 
* [cite_start]**Configurable Parameters:** Easily change the number of LEDs, GPIO pins, clock frequency, and global brightness in `led_test_c.c`. 
* **Animation and Test Patterns:**
    * [cite_start]A sequential lighting test to verify connectivity. 
    * [cite_start]A static color gradient. 
    * [cite_start]A yellow and cyan stripes pattern. 
    * [cite_start]A smooth, animated rainbow effect. 
* [cite_start]**Serial Debug Output:** The program prints its status and the currently running pattern to the serial monitor for easy debugging. 

## Hardware Requirements

* [cite_start]**Board:** Raspberry Pi Pico W 
* [cite_start]**LEDs:** SK9822 or APA102 LED Strip (configured for 144 LEDs). 
* **Wiring:**
    * [cite_start]LED Data Input -> Pico GPIO 11 
    * [cite_start]LED Clock Input -> Pico GPIO 10 
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