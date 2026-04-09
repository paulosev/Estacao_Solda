#include "hal_display.h"
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
    u8g2.drawStr(x, y, txt);
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