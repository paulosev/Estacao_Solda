#include "ui.h"
#include "hal_display.h"
#include "hal_encoder.h"
#include "controle_temp.h"
#include <Arduino.h>

/*
 * ============================================================
 * VARIÁVEIS
 * ============================================================
 */

static float setpoint = 200;
static bool editando = false;

static uint32_t last_update = 0;
static uint32_t last_click = 0;

/*
 * ============================================================
 * INIT
 * ============================================================
 */
void ui_init()
{
    hal_display_init();
    hal_encoder_init();
}

/*
 * ============================================================
 * DESENHA BARRA DE POTÊNCIA
 * ============================================================
 */
static void draw_bar(int x, int y, int w, int h, int percent)
{
    int fill = (w * percent) / 100;

    // contorno
    hal_display_print_str(x, y - 2, "[");
    hal_display_print_str(x + w + 2, y - 2, "]");

    // preenchimento simples
    for (int i = 0; i < fill; i += 4)
    {
        hal_display_print_str(x + i, y, "|");
    }
}

/*
 * ============================================================
 * LOOP
 * ============================================================
 */
void ui_update()
{
    int8_t delta = hal_encoder_get_delta();

    if (editando)
    {
        setpoint += delta * 5;

        if (setpoint < 50) setpoint = 50;
        if (setpoint > 450) setpoint = 450;

        controle_set_temp(setpoint);
    }

    if (hal_encoder_pressed() && (millis() - last_click > 200))
    {
        last_click = millis();
        editando = !editando;
    }

    if (millis() - last_update < 100)
        return;

    last_update = millis();

    int temp = (int)controle_get_temp();
    int sp   = (int)setpoint;
    int pwr  = (int)controle_get_power();

    bool standby = controle_is_standby();

    hal_display_begin();

    do
    {
        /*
         * ----------- TEMP -----------
         */
        hal_display_font_large();
        hal_display_print_int(0, 30, temp);
        hal_display_print_str(60, 30, "C");

        /*
         * ----------- SETPOINT (com destaque) -----------
         */
        hal_display_font_small();

        if (editando)
            hal_display_print_str(0, 50, ">");
        else
            hal_display_print_str(0, 50, " ");

        hal_display_print_str(10, 50, "SET:");
        hal_display_print_int(45, 50, sp);
        hal_display_print_str(75, 50, "C");

        /*
         * ----------- POWER BAR -----------
         */
        draw_bar(0, 60, 80, 5, pwr);

        /*
         * ----------- STATUS -----------
         */
        if (standby)
            hal_display_print_str(90, 60, "STBY");
        else
            hal_display_print_str(90, 60, "RUN");

    } while (hal_display_next());
}