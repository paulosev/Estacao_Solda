#include "hal_encoder.h"
#include <Arduino.h>
#include "config.h"

// ============================================================
// ESTADO INTERNO
// ============================================================

static volatile int  accumulated   = 0;
static volatile int  last_clk      = HIGH;

static volatile bool btn_pressed   = false;
static volatile bool btn_last      = false;
static unsigned long btn_last_time = 0;

#define DEBOUNCE_MS 40

// ============================================================
// ISR — ENCODER (dispara em qualquer borda do CLK)
// ============================================================

// Conta apenas em uma das bordas E verifica o par CLK+DT
// para pular pulsos — divide por 2 a resolução

static void isr_encoder()
{
    int clk = digitalRead(PIN_ENC_CLK);
    if (clk != last_clk)
    {
        last_clk = clk;
        if (clk == LOW)   // só borda de descida
        {
            if (digitalRead(PIN_ENC_DT) == HIGH)
                accumulated++;
            else
                accumulated--;
        }
    }
}

// ============================================================
// ISR — BOTÃO (dispara na borda de descida = pressiona)
// ============================================================

static void isr_button()
{
    unsigned long now = millis();

    if (now - btn_last_time > DEBOUNCE_MS)
    {
        btn_last_time = now;
        btn_pressed   = true;
    }
}

// ============================================================
// INIT
// ============================================================

void hal_encoder_init()
{
    pinMode(PIN_ENC_CLK, INPUT);
    pinMode(PIN_ENC_DT,  INPUT);
    pinMode(PIN_ENC_SW,  INPUT);

    last_clk = digitalRead(PIN_ENC_CLK);

    // Interrupção no CLK — qualquer borda para pegar subida e descida
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), isr_encoder, CHANGE);

    // Interrupção no SW — apenas borda de descida (pressiona)
    // O capacitor de 100nf já faz o debounce de hardware
    attachInterrupt(digitalPinToInterrupt(PIN_ENC_SW), isr_button, FALLING);
}

// ============================================================
// UPDATE — apenas debounce do botão por software (redundância)
// Pode ser chamado no loop ou removido se o cap já bastar
// ============================================================

void hal_encoder_update()
{
    // Com interrupções o encoder não precisa de polling.
    // Função mantida para não quebrar a interface existente.
}

// ============================================================
// GET DELTA — consome e zera o acumulador (acesso atômico)
// ============================================================

int hal_encoder_get_delta()
{
    noInterrupts();
    int d       = accumulated;
    accumulated = 0;
    interrupts();
    return d;
}

// ============================================================
// PRESSED — consome e zera o flag
// ============================================================

bool hal_encoder_pressed()
{
    noInterrupts();
    bool p      = btn_pressed;
    btn_pressed = false;
    interrupts();
    return p;
}