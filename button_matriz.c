#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

// *** Configurações de Hardware ***
#define LED_RED_PIN     11    // LED vermelho do RGB
#define LED_GREEN_PIN   12    // LED verde do RGB (não utilizado neste exemplo)
#define LED_BLUE_PIN    13    // LED azul do RGB (não utilizado neste exemplo)

#define BUTTON_A_PIN     5    // Botão A
#define BUTTON_B_PIN     6    // Botão B

#define WS2812_PIN       7    // Pino conectado à cadeia de WS2812
#define NUM_LEDS         25   // Matriz 5x5

// *** Configurações Gerais ***
#define DEBOUNCE_DELAY_MS   200    // Tempo de debounce em milissegundos

// *** Variáveis Globais ***
volatile uint8_t current_digit = 0;      // Dígito atual (0 a 9)
volatile uint32_t last_interrupt_time_a = 0;
volatile uint32_t last_interrupt_time_b = 0;

// Array para armazenar as cores dos LEDs WS2812 (formato 0xRRGGBB)
uint32_t ws2812_leds[NUM_LEDS];

// Cor utilizada para os dígitos (por exemplo, um tom de verde)
#define DIGIT_COLOR   0x003200    // 0x00RRGGBB (verde moderado)
#define OFF_COLOR     0x000000

// *** Mapa dos Dígitos (0 a 9) em formato 5x5 ***
// Cada dígito é representado por uma matriz 5x5 de 0s (apagado) e 1s (aceso)
const uint8_t digit_patterns[10][5][5] = {
    // Dígito 0
    {
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,1,1,1,1}
    },
    // Dígito 1
    {
        {0,0,1,0,0},
        {0,1,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,1,1,1,0}
    },
    // Dígito 2
    {
        {1,1,1,1,1},
        {0,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,1,1,1,1}
    },
    // Dígito 3
    {
        {1,1,1,1,1},
        {0,0,0,0,1},
        {0,1,1,1,1},
        {0,0,0,0,1},
        {1,1,1,1,1}
    },
    // Dígito 4
    {
        {1,0,0,1,0},
        {1,0,0,1,0},
        {1,1,1,1,1},
        {0,0,0,1,0},
        {0,0,0,1,0}
    },
    // Dígito 5
    {
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,1,1,1,1},
        {0,0,0,0,1},
        {1,1,1,1,1}
    },
    // Dígito 6
    {
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1}
    },
    // Dígito 7
    {
        {1,1,1,1,1},
        {0,0,0,0,1},
        {0,0,0,1,0},
        {0,0,1,0,0},
        {0,1,0,0,0}
    },
    // Dígito 8
    {
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1}
    },
    // Dígito 9
    {
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1},
        {0,0,0,0,1},
        {1,1,1,1,1}
    }
};


// Protótipos das Funções
void display_digit(uint8_t digit);
void ws2812_update(void);  // Função que envia o array ws2812_leds para a cadeia de LEDs.
int64_t timer_callback(alarm_id_t id, void *user_data);
void gpio_callback(uint gpio, uint32_t events);

// Função principal
// ====================================================================
int main() {
    stdio_init_all();

    // Inicializa os pinos do LED RGB:
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, 0);
    // Se desejar, inicialize os pinos verde e azul:
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_put(LED_GREEN_PIN, 0);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    gpio_put(LED_BLUE_PIN, 0);

    // Inicializa os botões A e B com pull-up interno:
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configura interrupção para os botões (detecta borda de descida)
    // Usaremos uma única callback para ambos os pinos
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Inicializa a interface para os WS2812
    // Aqui assumimos que a função ws2812_program_init(WS2812_PIN) inicializa o pino e o PIO.
    // Você deverá incluir e configurar a implementação apropriada para sua aplicação.
    // Exemplo:
    // ws2812_program_init(WS2812_PIN);
    // Para este exemplo, assumiremos que a função ws2812_update() envia o array ws2812_leds.

    // Exibe o dígito inicial (0)
    display_digit(current_digit);

    // Configura um timer/alarm para piscar o LED vermelho.
    // O LED pisca a 5 Hz: período de 200 ms; alterna a cada 100 ms.
    add_alarm_in_ms(100, timer_callback, NULL, true);

    // Loop principal (as ações ocorrem via interrupções e timer)
    while (1) {
        tight_loop_contents();
    }
    return 0;
}

// ====================================================================
// Função para atualizar a matriz WS2812 de acordo com o dígito atual
// ====================================================================
void display_digit(uint8_t digit) {
    // Seleciona o padrão correspondente (caso digit > 9, usa o padrão do 0)
    const uint8_t (*pattern)[5] = digit_patterns[(digit < 10) ? digit : 0];

    // Mapeia cada posição da matriz 5x5 para o array de LEDs.
    // Supondo mapeamento simples: índice = linha * 5 + coluna.
    for (uint8_t row = 0; row < 5; row++) {
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t idx = row * 5 + col;
            if (pattern[row][col] == 1) {
                ws2812_leds[idx] = DIGIT_COLOR;
            } else {
                ws2812_leds[idx] = OFF_COLOR;
            }
        }
    }
    ws2812_update();
}

// Função fictícia que envia os dados para os LEDs WS2812
// Nesta função, você deverá implementar o envio dos dados para os LEDs,
// utilizando, por exemplo, um programa em PIO ou uma biblioteca existente.
// Para este exemplo, consideramos que a função já está implementada.
void ws2812_update(void) {
    // Exemplo: envia o array ws2812_leds para o pino WS2812_PIN.
    // Esta implementação depende do seu driver.
    // ...
}

// Função de callback do timer (pisca o LED vermelho)
int64_t timer_callback(alarm_id_t id, void *user_data) {
    // Inverte o estado do LED vermelho
    gpio_xor_mask(1u << LED_RED_PIN);
    // Retorna o tempo para o próximo callback (100 ms)
    return 100 * 1000;   // 100 ms em microssegundos
}

// Função de callback para as interrupções dos botões
void gpio_callback(uint gpio, uint32_t events) {
    // Obtém o tempo atual (em milissegundos)
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN) {
        // Verifica debounce para o botão A
        if ((current_time - last_interrupt_time_a) < DEBOUNCE_DELAY_MS)
            return;
        last_interrupt_time_a = current_time;
        // Incrementa o dígito (rotação de 9 para 0)
        current_digit = (current_digit + 1) % 10;
        display_digit(current_digit);
        // Para depuração: printf("Botão A pressionado. Novo dígito: %d\n", current_digit);
    }
    else if (gpio == BUTTON_B_PIN) {
        // Verifica debounce para o botão B
        if ((current_time - last_interrupt_time_b) < DEBOUNCE_DELAY_MS)
            return;
        last_interrupt_time_b = current_time;
        // Decrementa o dígito (rotação de 0 para 9)
        current_digit = (current_digit == 0) ? 9 : current_digit - 1;
        display_digit(current_digit);
        // Para depuração: printf("Botão B pressionado. Novo dígito: %d\n", current_digit);
    }
}