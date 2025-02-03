// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ------- //
// ws2818b //
// ------- //

#define ws2812b_wrap_target 0
#define ws2812b_wrap 3

static const uint16_t ws2812b_program_instructions[] = {
            //     .wrap_target
    0x6221, //  0: out    x, 1            side 0 [2] 
    0x1123, //  1: jmp    !x, 3           side 1 [1] 
    0x1400, //  2: jmp    0               side 1 [4] 
    0xa442, //  3: nop                    side 0 [4] 
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program ws2812b_program = {
    .instructions = ws2812b_program_instructions,
    .length = 4,
    .origin = -1,
};

static inline pio_sm_config ws2812b_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + ws2818b_wrap_target, offset + ws2818b_wrap);
    sm_config_set_sideset(&c, 1, false, false);
    return c;
}

#include "hardware/clocks.h"
void ws2812b_program_init(PIO pio, uint sm, uint offset, uint pin, float freq) {
  pio_gpio_init(pio, pin);
  pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
  // Program configuration.
  pio_sm_config c = ws2812b_program_get_default_config(offset);
  sm_config_set_sideset_pins(&c, pin); // Uses sideset pins.
  sm_config_set_out_shift(&c, true, true, 8); // 8 bit transfers, right-shift.
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX); // Use only TX FIFO.
  float prescaler = clock_get_hz(clk_sys) / (10.f * freq); // 10 cycles per transmission, freq is frequency of encoded bits.
  sm_config_set_clkdiv(&c, prescaler);
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true);
}

#endif

