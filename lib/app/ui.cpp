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

// Setpoint de temperatura
static float setpoint = 200;

// Indica se está em modo edição
static bool editando = false;

// Controle de tempo (evita atualização excessiva)
static uint32_t last_update = 0;

// Debounce do botão
static uint32_t last_click = 0;


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
 * LOOP PRINCIPAL DA UI
 * ============================================================
 */
void ui_update()
{
    /*
     * ================= ENCODER =================
     */

    int8_t delta = hal_encoder_get_delta();

    if (editando)
    {
        // Ajusta temperatura em passos de 5°C
        setpoint += delta * 5;

        // Limites de segurança
        if (setpoint < 50) setpoint = 50;
        if (setpoint > 450) setpoint = 450;

        // Atualiza controle
        controle_set_temp(setpoint);
    }

    /*
     * ================= BOTÃO =================
     */

    // Detecta clique com debounce não bloqueante
    if (hal_encoder_pressed() && (millis() - last_click > 200))
    {
        last_click = millis();
        editando = !editando;
    }

    /*
     * ================= ATUALIZAÇÃO =================
     */

    // Atualiza a cada 100ms
    if (millis() - last_update < 100)
        return;

    last_update = millis();

    float temp = controle_get_temp();

    char buffer[32];

    /*
     * ================= DESENHO (U8g2 PAGE MODE) =================
     */

    hal_display_begin();

    do
    {
        /*
         * ----------- TEMPERATURA (GRANDE) -----------
         */

        hal_display_font_large();

        sprintf(buffer, "%.0fC", temp);
        hal_display_print(0, 30, buffer);

        /*
         * ----------- SETPOINT -----------
         */

        hal_display_font_small();

        sprintf(buffer, "SET: %.0fC", setpoint);
        hal_display_print(0, 50, buffer);

        /*
         * ----------- MODO -----------
         */

        if (editando)
            hal_display_print(80, 50, "EDIT");
        else
            hal_display_print(80, 50, "RUN");

    } while (hal_display_next()); // <-- CORREÇÃO AQUI
}