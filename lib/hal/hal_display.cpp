#include "hal_display.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

/*
 * ============================================================
 * CONFIGURAÇÃO DO DISPLAY
 * ============================================================
 *
 * SSD1306 128x64 via I2C (hardware)
 *
 * _1_ = modo PAGE (baixo uso de RAM)
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
 * INÍCIO DO FRAME
 * ============================================================
 *
 * Deve ser chamado antes do loop de desenho
 */
void hal_display_begin()
{
    u8g2.firstPage();
}


/*
 * ============================================================
 * PRÓXIMA PÁGINA
 * ============================================================
 *
 * Retorna true enquanto ainda há páginas
 */
bool hal_display_next()
{
    return u8g2.nextPage();
}


/*
 * ============================================================
 * PRINT DE TEXTO
 * ============================================================
 */
void hal_display_print(int x, int y, const char* txt)
{
    u8g2.drawStr(x, y, txt);
}


/*
 * ============================================================
 * FONTES
 * ============================================================
 */

// Fonte pequena (boa para labels)
void hal_display_font_small()
{
    u8g2.setFont(u8g2_font_6x10_tf);
}

// Fonte grande (ótima para temperatura)
void hal_display_font_large()
{
    u8g2.setFont(u8g2_font_logisoso24_tf);
}