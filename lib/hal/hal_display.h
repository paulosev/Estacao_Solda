#pragma once

/*
 * ============================================================
 * HAL DISPLAY (U8g2 - modo page)
 * ============================================================
 *
 * Interface de abstração do display OLED
 * usando biblioteca U8g2 em modo PAGE
 *
 * IMPORTANTE:
 * No modo page, o desenho precisa ser feito dentro de:
 *
 * firstPage()
 * do {
 *    draw...
 * } while(nextPage());
 */

// Inicializa display
void hal_display_init();

// Inicia o frame (equivalente ao firstPage)
void hal_display_begin();

// Avança página (equivalente ao nextPage)
// Retorna true enquanto ainda há páginas para desenhar
bool hal_display_next();

// Escreve texto na tela
void hal_display_print(int x, int y, const char* txt);

// Seleciona fonte pequena
void hal_display_font_small();

// Seleciona fonte grande
void hal_display_font_large();