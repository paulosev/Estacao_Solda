#include "hal_display.h"
#include "config.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

/*
 * ============================================================
 * CONFIGURAÇÃO
 * ============================================================
 */

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


/*
 * ============================================================
 * INICIALIZAÇÃO
 * ============================================================
 */
void hal_display_init()
{
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000); // 400kHz (rápido e estável)

    u8g2.begin();
}


/*
 * ============================================================
 * CONTROLE DE FRAME
 * ============================================================
 */
void hal_display_begin()
{
    u8g2.firstPage();
}

bool hal_display_next()
{
    return u8g2.nextPage();
}


/*
 * ============================================================
 * PRINT
 * ============================================================
 */

// Texto simples
void hal_display_print_str(int x, int y, const char* txt)
{
    // Converte Y de "topo do glifo" para baseline que a U8g2 espera
    u8g2.drawStr(x, y + u8g2.getAscent(), txt);
}

// Inteiro direto (sem sprintf)
void hal_display_print_int(int x, int y, int value)
{
    u8g2.setCursor(x, y);
    u8g2.print(value);
}


/*
 * ============================================================
 * FONTES
 * ============================================================
 */

// Fonte pequena
void hal_display_font_small()
{
    u8g2.setFont(u8g2_font_6x10_tf);
}

// Fonte grande (temperatura)
void hal_display_font_large()
{
    u8g2.setFont(u8g2_font_logisoso24_tf);
}

/*
 * ============================================================
 * DESENHO
 * ============================================================
 */
void hal_display_draw_pixel(int x, int y)
{
    u8g2.drawPixel(x, y);
}

void hal_display_draw_line(int x1, int y1, int x2, int y2)
{
    u8g2.drawLine(x1, y1, x2, y2);
}

// Estes precisam ser adicionados ao hal_display.cpp

void hal_display_draw_hline(int x, int y, int w) {
    u8g2.drawHLine(x, y, w);
}
void hal_display_draw_vline(int x, int y, int h) {
    u8g2.drawVLine(x, y, h);
}
void hal_display_fill_rect(int x, int y, int w, int h) {
    u8g2.drawBox(x, y, w, h);
}
void hal_display_draw_rect(int x, int y, int w, int h) {
    u8g2.drawFrame(x, y, w, h);
}
void hal_display_draw_char(int x, int y, char c)
{
    // Y já vem como topo do glifo — soma ascent para converter para baseline
    u8g2.drawGlyph(x, y + u8g2.getAscent(), c);
}
void hal_display_set_font_scale(int s)
{
    switch(s) {
        case 1: u8g2.setFont(u8g2_font_6x10_tf);        break;
        case 4: u8g2.setFont(u8g2_font_profont22_tf);   break;
        case 8: u8g2.setFont(u8g2_font_logisoso38_tn);  break;  // antes era logisoso42_tf
        default: u8g2.setFont(u8g2_font_6x10_tf);       break;
    }
}
void hal_display_set_dimmed(bool d){
    // Dimmed real: XOR color — desenha em cinza alternando pixels
    // U8g2 não tem alpha, mas XOR sobre fundo preto cria efeito de meio-brilho
    u8g2.setDrawColor(d ? 2 : 1); // 2 = XOR, 1 = normal (branco)
}
void hal_display_set_color_inverted(bool inv){
    u8g2.setDrawColor(inv ? 0 : 1); // 0 = preto (apaga), 1 = branco
}
int hal_display_char_w()
{
    return u8g2.getMaxCharWidth();
}