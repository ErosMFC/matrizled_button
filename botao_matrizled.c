#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2812b.pio.h"

// Definição de hardware
#define LED_COUNT 25
#define LED_PIN 7
#define LED_R 11
#define LED_G 12
#define LED_B 13
#define BUTTON_A 5
#define BUTTON_B 6

// Estrutura de um LED GRB
typedef struct {
    uint8_t G, R, B;
} npLED_t;

// Buffer da matriz de LEDs
npLED_t leds[LED_COUNT];

// Máquina PIO para os LEDs
PIO np_pio;
uint sm;

// Variáveis globais para controle do número exibido
volatile int numero_atual = 0;

// Protótipos das funções
void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite();
void exibirNumero(int numero);
void piscarLED();
void debounce_button(uint gpio, uint32_t events);
int getIndex(int x, int y);

void npInit(uint pin) {
    uint offset = pio_add_program(pio0);
    np_pio = pio0;

    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);
    }

    ws2812b_program_init(np_pio, sm, offset, pin, 800000.f);
    npClear();
}

void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

int getIndex(int x, int y) {
    return (y % 2 == 0) ? (y * 5 + x) : (y * 5 + (4 - x));
}

void exibirNumero(int numero) {
    npClear();
    
    const int numeros[10][25] = {
        {1,1,1,1,1, 1,0,0,0,1, 1,0,0,0,1, 1,0,0,0,1, 1,1,1,1,1}, // 0
        {0,0,1,0,0, 0,1,1,0,0, 1,0,1,0,0, 0,0,1,0,0, 1,1,1,1,1}, // 1
        {1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1}, // 2
        {1,1,1,1,1, 0,0,0,0,1, 0,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 3
        {1,0,0,1,1, 1,0,0,1,0, 1,1,1,1,1, 0,0,0,1,0, 0,0,0,1,1}, // 4
        {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}, // 5
        {1,1,1,1,1, 1,0,0,0,0, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 6
        {1,1,1,1,1, 0,0,0,0,1, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0}, // 7
        {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1}, // 8
        {1,1,1,1,1, 1,0,0,0,1, 1,1,1,1,1, 0,0,0,0,1, 1,1,1,1,1}  // 9
    };

    for (int i = 0; i < LED_COUNT; i++) {
        if (numeros[numero][i]) {
            npSetLED(i, 255, 0, 0);
        }
    }

    npWrite();
}

void debounce_button(uint gpio, uint32_t events) {
    sleep_ms(50); 
    if (gpio_get(gpio) == 0) { 
        if (gpio == BUTTON_A) {
            numero_atual = (numero_atual + 1) % 10;
        } else if (gpio == BUTTON_B) {
            numero_atual = (numero_atual + 9) % 10;
        }
        exibirNumero(numero_atual);
    }
}

void piscarLED() {
    static bool estado = false;
    estado = !estado;
    gpio_put(LED_R, estado);
}

int main() {
    stdio_init_all();

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_edge_fall(BUTTON_A, true);
    gpio_set_irq_callback(debounce_button);
    irq_set_enabled(IO_IRQ_BANK0, true);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_edge_fall(BUTTON_B, true);

    npInit(LED_PIN);
    exibirNumero(0);

    repeating_timer_t timer;
    add_repeating_timer_ms(-200, (repeating_timer_callback_t) piscarLED, NULL, &timer);

    while (true) {
        tight_loop_contents();
    }
}