#pragma once

/*
 * ============================================================
 * UI (INTERFACE DO USUÁRIO)
 * ============================================================
 *
 * Responsável por:
 *  - leitura do encoder
 *  - controle de menus
 *  - atualização do display
 */

// Inicializa UI
void ui_init();

// Atualiza UI (chamar no loop)
void ui_update();