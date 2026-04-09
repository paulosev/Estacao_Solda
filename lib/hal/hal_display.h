#pragma once

/*
 * ============================================================
 * HAL DISPLAY
 * ============================================================
 *
 * Camada de abstração do display OLED (SSD1306)
 * Responsável por:
 *  - Inicialização do display
 *  - Escrita de texto
 *  - Atualização da tela
 *
 * Isso desacopla a aplicação da biblioteca gráfica
 */

// Inicializa o display (I2C + SSD1306)
void hal_display_init();

// Limpa o buffer interno (não atualiza a tela ainda)
void hal_display_clear();

// Envia o buffer para o display físico
void hal_display_update();

// Escreve texto em posição (x,y)
void hal_display_print(int x, int y, const char* txt);