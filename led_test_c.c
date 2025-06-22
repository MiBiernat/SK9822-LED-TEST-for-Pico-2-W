#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"

// Dołącz wygenerowany nagłówek z nowego programu PIO
#include "sk9822.pio.h"

// --- Konfiguracja Diód ---
#define NUM_LEDS 144
#define DATA_PIN 11
#define CLOCK_PIN 10

// Zalecana bezpieczna prędkość zegara dla SK9822/APA102 to 15-25 MHz
#define LED_CLOCK_FREQ (15 * 1000 * 1000)

// Domyślna globalna jasność (0-31)
#define GLOBAL_BRIGHTNESS_VALUE 10

// --- Konfiguracja Bufora Ramki SK9822 ---
// Ramka startowa (32 bity zer)
#define START_FRAME_COUNT 1

// Ramka końcowa (32 bity jedynek)
// Dla SK9822/APA102, po wysłaniu danych potrzeba dodatkowych NUM_LEDS/2 cykli zegara.
// Dla 144 diod to 72 cykle. Każda ramka końcowa to 32 cykle.
// ceil(72 / 32) = 3 ramki końcowe.
#define END_FRAME_COUNT 3

// Całkowity rozmiar bufora w słowach 32-bitowych
#define FRAME_BUFFER_SIZE (START_FRAME_COUNT + NUM_LEDS + END_FRAME_COUNT)

// Bufor na ramki danych (teraz jako uint32_t)
uint32_t led_frame_buffer[FRAME_BUFFER_SIZE];

// Zmienne DMA
int dma_chan;
PIO pio_instance = pio0;
uint sm;

// Funkcja tworząca 32-bitową ramkę dla diody SK9822
// Format: 111 (3b) + Jasność (5b) + Niebieski (8b) + Zielony (8b) + Czerwony (8b)
static inline uint32_t create_sk9822_frame(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    // Ogranicz jasność do 5 bitów (0-31)
    uint32_t bright_val = brightness & 0x1F;
    
    return (0b111 << 29) |        // 3 bity startowe '111'
           (bright_val << 24) |   // 5 bitów jasności
           ((uint32_t)b << 16) |  // 8 bitów - Niebieski
           ((uint32_t)g << 8) |   // 8 bitów - Zielony
           (uint32_t)r;           // 8 bitów - Czerwony
}

// Ustaw kolor pojedynczej diody w buforze
void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    if (index < 0 || index >= NUM_LEDS) return;
    // Pozycja w buforze: 1 (start) + indeks diody
    led_frame_buffer[START_FRAME_COUNT + index] = create_sk9822_frame(r, g, b, brightness);
}

// Ustaw ten sam kolor dla wszystkich diod
void set_all_leds_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    for (int i = 0; i < NUM_LEDS; ++i) {
        set_led_color(i, r, g, b, brightness);
    }
}

// Inicjalizacja bufora (ramki startowe i końcowe)
void prepare_frames() {
    // Ustaw ramkę startową (32 bity zer)
    led_frame_buffer[0] = 0x00000000;

    // Ustaw ramki końcowe (32 bity jedynek)
    int end_frame_offset = START_FRAME_COUNT + NUM_LEDS;
    for (int i = 0; i < END_FRAME_COUNT; ++i) {
        led_frame_buffer[end_frame_offset + i] = 0xFFFFFFFF;
    }
}

// Dodaj flagi dla bezpiecznego obsługiwania bufora
volatile bool dma_transfer_in_progress = false;

// Zmodyfikowana funkcja wysyłania bufora
void send_frame_buffer() {
    // Czekaj na zakończenie poprzedniego transferu przed rozpoczęciem nowego
    dma_channel_wait_for_finish_blocking(dma_chan);
    
    // Ustaw flagę aktywnego transferu
    dma_transfer_in_progress = true;
    
    // Konfiguruje i uruchamia transfer DMA
    dma_channel_transfer_from_buffer_now(dma_chan, led_frame_buffer, FRAME_BUFFER_SIZE);
    
    // Opcjonalnie: dla animacji pozwalamy na działanie w tle
    // Dla zmian wzorów lepiej poczekać, aby uniknąć uszkodzenia bufora
}

// Główna funkcja inicjalizacyjna PIO i DMA
void sk9822_init() {
    sm = pio_claim_unused_sm(pio_instance, true);
    uint offset = pio_add_program(pio_instance, &sk9822_program);

    // Wywołaj funkcję inicjalizacyjną z pliku .pio
    sk9822_program_init(pio_instance, sm, offset, LED_CLOCK_FREQ, CLOCK_PIN, DATA_PIN);

    // Inicjalizacja kanału DMA
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, pio_get_dreq(pio_instance, sm, true));
    
    // Dodaj obsługę przerwań DMA (opcjonalnie)
    dma_channel_set_irq0_enabled(dma_chan, true);
    
    dma_channel_configure(dma_chan, &c, &pio_instance->txf[sm], NULL, 0, false);
    
    // Inicjalizuj flagi
    dma_transfer_in_progress = false;
}


void rainbow_effect(float phase) {
    for (int i = 0; i < NUM_LEDS; ++i) {
        float hue = (float)i / NUM_LEDS * 6.28f + phase;
        uint8_t r = (uint8_t)(127 * (1 + sin(hue)));
        uint8_t g = (uint8_t)(127 * (1 + sin(hue + 2.09f))); // 2*PI/3
        uint8_t b = (uint8_t)(127 * (1 + sin(hue + 4.18f))); // 4*PI/3
        set_led_color(i, r, g, b, GLOBAL_BRIGHTNESS_VALUE);
    }
}

void pattern_gradient() {
    printf("\n--- Wzór: Gradient kolorów ---\n");
    // Czekaj na zakończenie poprzedniego transferu przed modyfikacją bufora
    dma_channel_wait_for_finish_blocking(dma_chan);
    
    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t r = (uint8_t)(255 * i / NUM_LEDS);
        uint8_t g = (uint8_t)(255 - (255 * i / NUM_LEDS));
        uint8_t b = 128;
        set_led_color(i, r, g, b, GLOBAL_BRIGHTNESS_VALUE);
    }
    send_frame_buffer();
    sleep_ms(5000); // Zwiększono z 2000 do 5000 ms
}

void pattern_stripes() {
    printf("\n--- Wzór: Paski ---\n");
    // Czekaj na zakończenie poprzedniego transferu przed modyfikacją bufora
    dma_channel_wait_for_finish_blocking(dma_chan);
    
    for (int i = 0; i < NUM_LEDS; i++) {
        if ((i / 12) % 2 == 0) {
            set_led_color(i, 255, 255, 0, GLOBAL_BRIGHTNESS_VALUE); // Żółty
        } else {
            set_led_color(i, 0, 255, 255, GLOBAL_BRIGHTNESS_VALUE); // Cyan
        }
    }
    send_frame_buffer();
    sleep_ms(5000); // Zwiększono z 2000 do 5000 ms
}


void pattern_rainbow_anim() {
    printf("\n--- Wzór: Animowana tęcza ---\n");
    for (int cycle = 0; cycle < 100; ++cycle) {
        // Czekaj aż poprzedni transfer DMA się zakończy
        dma_channel_wait_for_finish_blocking(dma_chan);
        
        // Teraz bezpiecznie możemy modyfikować bufor
        float phase = (float)cycle * 0.1f;
        rainbow_effect(phase);
        
        // Wysyłamy zaktualizowany bufor
        send_frame_buffer();
        
        // Synchronizacja czasowa
        sleep_ms(100);
    }
}

void pattern_all_off() {
    // Czekaj na zakończenie poprzedniego transferu przed modyfikacją bufora
    dma_channel_wait_for_finish_blocking(dma_chan);
    
    set_all_leds_color(0, 0, 0, 0);
    send_frame_buffer();
    sleep_ms(1000); // Zwiększono z 500 do 1000 ms
}


int main() {
    stdio_init_all();
    sleep_ms(3000); // Czas na otwarcie terminala
    printf("--- Start programu testu LED SK9822 ---\n");

    // Inicjalizacja wbudowanej diody LED na Pico
    #ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    #endif

    printf("Inicjalizacja PIO i DMA...\n");
    sk9822_init();
    
    printf("Przygotowanie bufora ramki...\n");
    prepare_frames();

    // --- Uruchomienie sekwencji testowej ---

    // Test: Zapalanie kolejnych czerwonych diod
    printf("\n--- Test: zapalanie kolejnych LEDów ---\n");
    for (int n = 1; n <= NUM_LEDS; n+=5) {
        set_all_leds_color(0, 0, 0, 0);
        for (int i = 0; i < n; ++i) {
            set_led_color(i, 255, 0, 0, GLOBAL_BRIGHTNESS_VALUE); // czerwony
        }
        send_frame_buffer();
        printf("Zapalono %d LED\n", n);
        dma_channel_wait_for_finish_blocking(dma_chan);
        sleep_ms(500); // Zwiększono z 100 do 500 ms
    }
    pattern_all_off();
    
    // Testy wzorów
    pattern_gradient();
    pattern_all_off();

    pattern_stripes();
    pattern_all_off();
    
    // Animowana tęcza
    pattern_rainbow_anim();
    pattern_all_off();

    printf("\nTesty zakończone. Program w pętli.\n");

    // Pętla główna - miganie diodą na płytce
    int counter = 0;
    while (true) {
        printf("Pętla główna, licznik: %d\n", counter++);
        #ifdef PICO_DEFAULT_LED_PIN
        gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
        #endif
        sleep_ms(1000);
    }
    return 0;
}
// Koniec pliku led_test_c.c
