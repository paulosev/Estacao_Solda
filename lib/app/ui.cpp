#include "ui.h"
#include "hal_display.h"
#include "hal_encoder.h"
#include "controle_temp.h"
#include <Arduino.h>

/*
 * ============================================================
 * ESTADOS DA UI
 * ============================================================
 */

enum UIState
{
    UI_RUN,
    UI_MENU,
    UI_GRAPH
};

static UIState state = UI_RUN;

/*
 * ============================================================
 * VARIÁVEIS
 * ============================================================
 */

static float setpoint = 200;
static int menu_index = 0;

static uint32_t last_click = 0;

/*
 * ============================================================
 * BUFFER DO GRÁFICO
 * ============================================================
 */

#define GRAPH_SIZE 128
static uint8_t graph[GRAPH_SIZE];
static uint8_t graph_index = 0;

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
 * ATUALIZA BUFFER DO GRÁFICO
 * ============================================================
 */
static void graph_add(float temp)
{
    int v = (int)temp;

    if (v < 0) v = 0;
    if (v > 255) v = 255;

    graph[graph_index++] = v;

    if (graph_index >= GRAPH_SIZE)
        graph_index = 0;
}

/*
 * ============================================================
 * DESENHA GRÁFICO
 * ============================================================
 */
static void draw_graph()
{
    for (int i = 0; i < GRAPH_SIZE - 1; i++)
    {
        int idx = (graph_index + i) % GRAPH_SIZE;
        int idx2 = (graph_index + i + 1) % GRAPH_SIZE;

        int y1 = 63 - (graph[idx] / 4);
        int y2 = 63 - (graph[idx2] / 4);

        hal_display_draw_line(i, y1, i + 1, y2);
    }
}

/*
 * ============================================================
 * TELA PRINCIPAL (RUN)
 * ============================================================
 */
static void tela_run()
{
    int temp = (int)controle_get_temp();
    int sp   = (int)setpoint;
    int pwr  = (int)controle_get_power();

    hal_display_font_large();
    hal_display_print_int(0, 30, temp);
    hal_display_print_str(60, 30, "C");

    hal_display_font_small();
    hal_display_print_str(0, 50, "SET:");
    hal_display_print_int(40, 50, sp);

    // barra de potência simples
    int bar = (pwr * 100) / 100;
    for (int i = 0; i < bar; i += 5)
        hal_display_draw_pixel(i, 60);
}

/*
 * ============================================================
 * MENU
 * ============================================================
 */

const char* menu_items[] =
{
    "Set Temp",
    "Grafico",
    "Voltar"
};

#define MENU_SIZE 3

static void tela_menu()
{
    hal_display_font_small();

    for (int i = 0; i < MENU_SIZE; i++)
    {
        if (i == menu_index)
            hal_display_print_str(0, 10 + i * 12, ">");
        else
            hal_display_print_str(0, 10 + i * 12, " ");

        hal_display_print_str(10, 10 + i * 12, menu_items[i]);
    }
}

/*
 * ============================================================
 * GRÁFICO
 * ============================================================
 */
static void tela_graph()
{
    draw_graph();
}

/*
 * ============================================================
 * INPUT (encoder)
 * ============================================================
 */
static void handle_input()
{
    int8_t delta = hal_encoder_get_delta();

    if (state == UI_RUN)
    {
        setpoint += delta * 5;

        if (setpoint < 50) setpoint = 50;
        if (setpoint > 450) setpoint = 450;

        controle_set_temp(setpoint);
    }
    else if (state == UI_MENU)
    {
        menu_index += delta;

        if (menu_index < 0) menu_index = 0;
        if (menu_index >= MENU_SIZE) menu_index = MENU_SIZE - 1;
    }

    // clique
    if (hal_encoder_pressed() && millis() - last_click > 200)
    {
        last_click = millis();

        switch (state)
        {
            case UI_RUN:
                state = UI_MENU;
                break;

            case UI_MENU:
                if (menu_index == 0)
                    state = UI_RUN;
                else if (menu_index == 1)
                    state = UI_GRAPH;
                else
                    state = UI_RUN;
                break;

            case UI_GRAPH:
                state = UI_RUN;
                break;
        }
    }
}

/*
 * ============================================================
 * LOOP
 * ============================================================
 */
void ui_update()
{
    handle_input();

    // atualiza gráfico
    graph_add(controle_get_temp());

    hal_display_begin();

    do
    {
        switch (state)
        {
            case UI_RUN:
                tela_run();
                break;

            case UI_MENU:
                tela_menu();
                break;

            case UI_GRAPH:
                tela_graph();
                break;
        }

    } while (hal_display_next());
}