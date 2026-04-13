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
// Inicializa o encoder (chamado no setup)
void hal_encoder_init();
//  Atualiza o estado do encoder (chamado no loop principal)
void hal_encoder_update();
// Retorna o incremento/decremento acumulado desde a última chamada (consome o evento)
int  hal_encoder_get_delta();
// Verifica se o botão do encoder foi pressionado (consome o evento)
bool hal_encoder_pressed();
// Verifica se o botão do encoder está pressionado (sem consumir o evento)
bool hal_encoder_button_is_pressed();