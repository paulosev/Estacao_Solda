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

// Retorna:
// -1 → girou para esquerda
//  0 → sem movimento
// +1 → girou para direita
int8_t hal_encoder_get_delta();

// Retorna true se botão pressionado
bool hal_encoder_pressed();