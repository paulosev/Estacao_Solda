#pragma once
#include <stdint.h>

/*
 * ============================================================
 * HAL ENCODER (KY-040)
 * ============================================================
 *
 * Responsável por:
 *  - Ler rotação (incremento/decremento)
 *  - Detectar clique do botão
 */
// Inicializa pinos do encoder
void hal_encoder_init();
void hal_encoder_update();
int  hal_encoder_get_delta();
bool hal_encoder_pressed();