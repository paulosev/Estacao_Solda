#include "hal_encoder.h"
#include <Arduino.h>
#include "config.h"

// ============================================================
// ESTADO INTERNO
// ============================================================
static volatile int  accumulated = 0;
static volatile uint8_t last_state = 0;    // estado anterior (CLK:bit1, DT:bit0)
static volatile bool btn_pressed = false;

// Tabela de transição válida para encoder quadrature
// Cada entrada representa o incremento (+1, -1 ou 0) para uma transição
// O índice é formado por: (estado_anterior << 2) | estado_atual
// Estados: bit1=CLK, bit0=DT
static const int8_t enc_table[16] = {
    0,  // 00 -> 00
    -1, // 00 -> 01
    1,  // 00 -> 10
    0,  // 00 -> 11 (inválido)
    1,  // 01 -> 00
    0,  // 01 -> 01
    0,  // 01 -> 10 (inválido)
    -1, // 01 -> 11
    -1, // 10 -> 00
    0,  // 10 -> 01 (inválido)
    0,  // 10 -> 10
    1,  // 10 -> 11
    0,  // 11 -> 00 (inválido)
    1,  // 11 -> 01
    -1, // 11 -> 10
    0   // 11 -> 11
};

// ============================================================
// ISR — ENCODER (dispara em CHANGE de CLK e DT)
// ============================================================
static void isr_encoder()
{
    // Lê os dois pinos e monta o estado atual (2 bits)
    uint8_t clk = digitalRead(PIN_ENC_CLK);
    uint8_t dt  = digitalRead(PIN_ENC_DT);
    uint8_t state = (clk << 1) | dt;

    // Calcula índice da tabela: (last_state << 2) | state
    uint8_t idx = (last_state << 2) | state;
    int8_t delta = enc_table[idx];
    if (delta != 0) {
        accumulated += delta;
    }
    last_state = state;
}

// ============================================================
// ISR — BOTÃO (dispara na borda de descida)
// ============================================================
static void isr_button()
{
    btn_pressed = true;
}

// ============================================================
// INIT
// ============================================================
void hal_encoder_init()
{
    pinMode(PIN_ENC_CLK, INPUT);
    pinMode(PIN_ENC_DT,  INPUT);
    pinMode(PIN_ENC_SW,  INPUT);

    // Lê estado inicial
    uint8_t clk = digitalRead(PIN_ENC_CLK);
    uint8_t dt  = digitalRead(PIN_ENC_DT);
    last_state = (clk << 1) | dt;

    // Interrupções em AMBOS os pinos, em QUALQUER borda
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), isr_encoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_DT),  isr_encoder, CHANGE);

    // Botão na borda de descida
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_SW), isr_button, FALLING);
}

void hal_encoder_update()
{
    // Não utilizada
}

int hal_encoder_get_delta()
{
    noInterrupts();
    int d = accumulated;
    accumulated = 0;
    interrupts();
    return d;
}

bool hal_encoder_pressed()
{
    noInterrupts();
    bool p = btn_pressed;
    btn_pressed = false;
    interrupts();
    return p;
}

bool hal_encoder_button_is_pressed()
{
    return digitalRead(PIN_ENC_SW) == LOW;
}