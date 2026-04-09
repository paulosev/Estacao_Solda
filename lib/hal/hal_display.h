#pragma once

/*
 * ============================================================
 * HAL DISPLAY (U8g2 - modo page)
 * ============================================================
 *
 * Interface de abstração do display OLED
 * com suporte a:
 *  - texto
 *  - números inteiros (sem sprintf!)
 */

// Inicializa display
void hal_display_init();

// Controle de frame (U8g2 page mode)
void hal_display_begin();
bool hal_display_next();

// Texto
void hal_display_print_str(int x, int y, const char* txt);

// Inteiro (sem conversão manual)
void hal_display_print_int(int x, int y, int value);

// Fontes
void hal_display_font_small();
void hal_display_font_large();

// Desenho de formas
void hal_display_draw_pixel(int x, int y);

// Desenho de linha (Bresenham)
void hal_display_draw_line(int x1, int y1, int x2, int y2);