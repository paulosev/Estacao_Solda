#pragma once
#include <Arduino.h>

/*
 * Camada HAL (Hardware Abstraction Layer)
 * Isola todo acesso direto ao hardware.
 */

// Inicializa GPIO, ADC, PWM
void hal_io_init();

// Controle do TRIAC (heater) via MOC3443
void hal_triac_write(bool estado);

// Controle do FAN — duty: 0–1000 (permil)
void hal_fan_write(uint16_t duty);

// Leituras ADC (retorna valor bruto 0–4095)
uint16_t hal_adc_termopar();
uint16_t hal_adc_ntc();

// Leitura da chave magnética (true = pistola encaixada = ligado)
bool hal_chave_ligada();
