#include "ui.h"
#include "hal_display.h"
#include "hal_encoder.h"
#include "controle_temp.h"
#include <Arduino.h>
#include <stdio.h>

/*
 * ============================================================
 * VARIÁVEIS DE ESTADO
 * ============================================================
 */

// Temperatura alvo (setpoint)
static float setpoint = 200;

// Define se estamos editando o setpoint
static bool editando = false;

// Controle de atualização da tela (evita flicker)
static uint32_t last_update = 0;


/*
 * ============================================================
 * INICIALIZAÇÃO
 * ============================================================
 */
void ui_init()
{
    hal_display_init();
    hal_encoder_init();
}


/*
 * ============================================================
 * LOOP DA INTERFACE
 * ============================================================
 */
void ui_update()
{
    /*
     * ================= LEITURA DO ENCODER =================
     */

    int8_t delta = hal_encoder_get_delta();

    // Se estiver em modo edição, altera o setpoint
    if (editando)
    {
        setpoint += delta * 5;

        // Limites de segurança
        if (setpoint < 50) setpoint = 50;
        if (setpoint > 450) setpoint = 450;

        // Atualiza controle térmico
        controle_set_temp(setpoint);
    }

    /*
     * ================= BOTÃO =================
     */

    if (hal_encoder_pressed())
    {
        delay(200); // debounce simples

        // Alterna entre modo RUN e AJUSTE
        editando = !editando;
    }

    /*
     * ================= ATUALIZAÇÃO DO DISPLAY =================
     */

    // Atualiza a cada 200 ms
    if (millis() - last_update < 200)
        return;

    last_update = millis();

    float temp = controle_get_temp();

    char buffer[32];

    hal_display_clear();

    /*
     * Linha 1: temperatura atual
     */
    sprintf(buffer, "Temp: %.1f C", temp);
    hal_display_print(0, 0, buffer);

    /*
     * Linha 2: setpoint
     */
    sprintf(buffer, "Set : %.0f C", setpoint);
    hal_display_print(0, 16, buffer);

    /*
     * Linha 3: estado do sistema
     */
    if (editando)
        hal_display_print(0, 32, "Modo: AJUSTE");
    else
        hal_display_print(0, 32, "Modo: RUN");

    /*
     * Envia buffer para o display
     */
    hal_display_update();
}