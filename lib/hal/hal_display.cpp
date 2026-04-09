#include "hal_display.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*
 * ============================================================
 * CONFIGURAÇÃO DO DISPLAY
 * ============================================================
 */

// Resolução do OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Endereço I2C padrão (pode variar: 0x3C ou 0x3D)
#define OLED_ADDR 0x3C

/*
 * Objeto global do display
 * Usa I2C (Wire) e sem pino de reset (-1)
 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


/*
 * ============================================================
 * INICIALIZAÇÃO
 * ============================================================
 */
void hal_display_init()
{
    // Inicializa barramento I2C
    Wire.begin();

    // Inicializa o display
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

    // Limpa buffer interno
    display.clearDisplay();

    // Envia tela limpa
    display.display();
}


/*
 * ============================================================
 * LIMPA BUFFER
 * ============================================================
 *
 * Importante:
 * - Não limpa diretamente o display físico
 * - Apenas o buffer de memória
 */
void hal_display_clear()
{
    display.clearDisplay();
}


/*
 * ============================================================
 * ATUALIZA DISPLAY
 * ============================================================
 *
 * Envia o conteúdo do buffer para o display
 */
void hal_display_update()
{
    display.display();
}


/*
 * ============================================================
 * PRINT DE TEXTO
 * ============================================================
 *
 * x, y → posição em pixels
 */
void hal_display_print(int x, int y, const char* txt)
{
    display.setCursor(x, y);

    // Tamanho da fonte (1 = padrão)
    display.setTextSize(1);

    // Cor branca (OLED monocromático)
    display.setTextColor(SSD1306_WHITE);

    display.print(txt);
}