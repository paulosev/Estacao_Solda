#include "ui.h"
#include "hal_display.h"
#include "hal_encoder.h"
#include "controle_temp.h"
#include <Arduino.h>
#include <stdint.h> 

// ============================================================
// ESTADOS
// ============================================================
enum UIState { UI_RUN, UI_EDIT_SP, UI_MENU, UI_GRAPH, UI_STANDBY };
static UIState state     = UI_RUN;
static UIState prev_state = UI_RUN;

// ============================================================
// VARIÁVEIS
// ============================================================
static float    setpoint    = 250.0f;
static int      menu_index  = 0;
static bool     sp_blink    = false;
static uint32_t last_click  = 0;
static uint32_t last_blink  = 0;
static uint32_t standby_timer = 0;   // ms sem interação

// ============================================================
// BUFFER DO GRÁFICO
// ============================================================
#define GRAPH_W  105
#define GRAPH_X   20
#define GRAPH_Y    8
#define GRAPH_H   44
#define GRAPH_TMAX 400

static uint8_t  g_buf[GRAPH_W];
static uint8_t  g_idx = 0;

static void graph_push(float t)
{
    int v = (int)t;
    if (v < 0)   v = 0;
    if (v > 255) v = 255;
    g_buf[g_idx] = (uint8_t)v;
    g_idx = (g_idx + 1) % GRAPH_W;
}

// ============================================================
// MICRO-PRIMITIVAS  (wrappers finos sobre hal_display)
// ============================================================

// Linha horizontal sólida
static inline void ui_hline(int16_t x, int16_t y, int16_t w)
{
    hal_display_draw_hline(x, y, w);
}

// Linha vertical sólida
static inline void ui_vline(int16_t x, int16_t y, int16_t h)
{
    hal_display_draw_vline(x, y, h);
}

// Borda de retângulo
static void ui_rect(int16_t x, int16_t y, int16_t w, int16_t h)
{
    ui_hline(x,     y,         w);
    ui_hline(x,     y + h - 1, w);
    ui_vline(x,     y,         h);
    ui_vline(x + w - 1, y,     h);
}

// Retângulo preenchido
static inline void ui_fill(int16_t x, int16_t y, int16_t w, int16_t h)
{
    hal_display_fill_rect(x, y, w, h);
}

// Linha tracejada horizontal
static void ui_dashed_hline(int16_t x, int16_t y, int16_t w,
                             uint8_t on, uint8_t off)
{
    for (int16_t i = 0; i < w; i += on + off)
    {
        int16_t len = ((i + on) < w) ? on : (w - i);
        ui_hline(x + i, y, len);
    }
}

// Barra de segmentos (val 0..100, segs segmentos, sw×sh px cada, gap px entre)
static void ui_bar(int16_t x, int16_t y, int val,
                   int total, uint8_t segs,
                   uint8_t sw, uint8_t sh, uint8_t gap)
{
    int filled = (val * segs) / total;
    for (int i = 0; i < segs; i++)
    {
        int16_t bx = x + i * (sw + gap);
        if (i < filled)
        {
            ui_fill(bx, y, sw, sh);
        }
        else
        {
            // Apenas os 4 cantos — segmento vazio
            hal_display_draw_pixel(bx,          y);
            hal_display_draw_pixel(bx + sw - 1, y);
            hal_display_draw_pixel(bx,          y + sh - 1);
            hal_display_draw_pixel(bx + sw - 1, y + sh - 1);
        }
    }
}

// ============================================================
// NÚMERO N DÍGITOS sem sprintf
// ============================================================
static void ui_print_int_pad(int x, int y, int val, int digits, int scale)
{
    char buf[8];
    int v = (val < 0) ? 0 : val;
    for (int i = digits - 1; i >= 0; i--)
    {
        buf[i] = '0' + (v % 10);
        v /= 10;
    }
    buf[digits] = '\0';

    // Seta a fonte UMA vez antes de medir
    hal_display_set_font_scale(scale);

    // Mede largura do char COM a fonte correta já ativa
    int cw = hal_display_char_w();

    // Espaçamento: largura do char + 1px de gap, tudo em escala 1
    // (as fontes já têm tamanho real — não multiplicar por scale)
    int step = cw + 1;

    for (int i = 0; i < digits; i++)
    {
        hal_display_draw_char(x + i * step, y, buf[i]);
    }

    // Restaura fonte pequena
    hal_display_set_font_scale(1);
}

// Imprime string simples (escala 1)
static void ui_str(int x, int y, const char* s)
{
    hal_display_font_small();
    hal_display_print_str(x, y, s); // print_str já soma ascent internamente
}

// ============================================================
// TELA RUN  — temperatura ENORME
// ============================================================
//
//  ┌─────────────────────────────────┐
//  │  2 5 0            R E A D Y     │  ← linha 0..50
//  │  (escala 8×)                    │
//  │                                 │
//  ├─────────────────────────────────┤  ← y=52
//  │ SP:250C │P:████░░│F:███░       │  ← y=53..63
//  └─────────────────────────────────┘
//
static void tela_run()
{
    int temp = (int)controle_get_temp();
    int sp   = (int)setpoint;
    int pwr  = (int)controle_get_power();    // 0..100
    int fan  = (int)controle_get_fan();      // 0..100

    // ── Temperatura central (escala 8) ──────────────────────
    ui_print_int_pad(2, 4, temp, 3, 8);

    // "C" em escala 4, topo-direita da temp
    hal_display_set_font_scale(4);
    hal_display_draw_char(2 + 3 * (hal_display_char_w() + 1) * 8 + 4, 4, 'C');
    hal_display_set_font_scale(1);

    // ── Status: READY / HEAT / COOL ─────────────────────────
    int diff = temp - sp;
    hal_display_font_small();
    if (diff > 5)
    {
        ui_str(92, 1, "COOL");
    }
    else if (diff < -5)
    {
        ui_str(92, 1, "HEAT");
    }
    else
    {
        ui_str(89, 1, "READY");
    }

    // ── Separador ───────────────────────────────────────────
    ui_hline(0, 52, 128);

    // ── Rodapé comprimido ────────────────────────────────────
    // SP
    ui_str(1, 55, "SP");
    ui_print_int_pad(13, 55, sp, 3, 1);
    hal_display_draw_char(28, 55, 'C');

    // Divisor
    hal_display_set_dimmed(true);
    ui_vline(41, 53, 11);
    hal_display_set_dimmed(false);

    // Potência
    hal_display_draw_char(43, 55, 'P');
    ui_bar(50, 55, pwr, 100, 6, 4, 5, 1);   // 6 seg × 5px = 54px

    // Divisor
    hal_display_set_dimmed(true);
    ui_vline(87, 53, 11);
    hal_display_set_dimmed(false);

    // Fan
    hal_display_draw_char(89, 55, 'F');
    ui_bar(96, 55, fan, 100, 4, 4, 5, 1);   // 4 seg
}

// ============================================================
// TELA EDIT SETPOINT
// ============================================================
//
//  ┌─────────────────────────────────┐
//  │ SET TEMP                        │
//  ├─────────────────────────────────┤
//  │      ╔═══════╗   ▲              │
//  │      ║  2 7 5 C║   │              │
//  │      ╚═══════╝   ▼              │
//  ├─────────────────────────────────┤
//  │ NOW:187C          OK:CONF       │
//  └─────────────────────────────────┘
//
static void tela_edit_sp()
{
    int sp   = (int)setpoint;
    int temp = (int)controle_get_temp();

    // ── Cabeçalho ──────────────────────────────────────────
    ui_str(1, 1, "SET TEMP");
    ui_hline(0, 11, 128);

    // ── Setpoint grande (escala 8 = fonte logisoso42) ───────
    // Fonte logisoso42: char ~26px largura, ~42px altura
    // 3 dígitos + "C" (escala 4 ~14px) cabem em 128px
    const int SP_X  = 4;   // margem esquerda
    const int SP_Y  = 12;  // topo dos dígitos

    hal_display_set_font_scale(8);
    int cw8 = hal_display_char_w();          // largura real da fonte grande
    hal_display_set_font_scale(1);           // restaura para não sujar estado

    ui_print_int_pad(SP_X, SP_Y, sp, 3, 8); // desenha 3 dígitos em escala 8

    // "C" em escala 4, ao lado dos dígitos
    hal_display_set_font_scale(4);
    hal_display_draw_char(SP_X + 3 * (cw8 + 1) + 4, SP_Y, 'C');
    hal_display_set_font_scale(1);

    // ── Borda piscante ──────────────────────────────────────
    if (sp_blink)
    {
        int bx = SP_X - 3;
        int bw = 3 * (cw8 + 1) + 20;   // cobre os 3 dígitos + C
        int bh = 42;                     // altura da fonte grande
        ui_rect(bx, SP_Y - 1, bw, bh);
    }

    // ── Setas ▲ ▼ à direita ─────────────────────────────────
    hal_display_set_font_scale(8);
    int ax = SP_X + 3 * (cw8 + 1) + 24;
    hal_display_set_font_scale(1);

    int arrow_mid = SP_Y + 20;           // meio vertical da área
    for (int i = 0; i < 5; i++)          // ▲
        ui_hline(ax - i, arrow_mid - 8 + i, i * 2 + 1);
    for (int i = 0; i < 5; i++)          // ▼
        ui_hline(ax - (4 - i), arrow_mid + 4 + i, (4 - i) * 2 + 1);

    // ── Rodapé ──────────────────────────────────────────────
    ui_hline(0, 54, 128);
    ui_str(1, 56, "NOW:");
    ui_print_int_pad(26, 56, temp, 3, 1);
    hal_display_draw_char(44, 56, 'C');
    ui_str(72, 56, "OK:CONF");
}

// ============================================================
// MENU — foco fixo, lista rolante
// ============================================================

static const char* menu_items[] = {
    "SET TEMP",
    "GRAFICO",
    "PERFIS",
    "FAN",
    "STANDBY",
    "VOLTAR"
};
static const char* menu_hints[] = {
    "Ajusta setpoint",
    "Historico temp",
    "Perfis salvos",
    "Velocidade fan",
    "Modo espera",
    "Tela principal"
};
#define MENU_SIZE 6

// Posições fixas no display
#define MENU_AREA_Y   10   // início da área de itens (abaixo do header)
#define MENU_AREA_H   44   // altura da área (até o rodapé)
#define MENU_ROW_H    16   // altura de cada linha em pixels
#define MENU_FOCUS_Y  22   // y do topo do bloco de foco (centro da área)

static void tela_menu()
{
    // MENU_ROW_H = 16, fonte 6x10: ascent ~8px, altura total ~10px
    // Para centralizar na linha: text_y = linha_y + (16 - 10) / 2 = linha_y + 3

    hal_display_font_small();

    // ── Cabeçalho ──────────────────────────────────────────
    hal_display_set_dimmed(false);
    ui_str(2, 1, "MENU");
    ui_hline(0, 10, 128);

    // ── Bloco de foco fixo ──────────────────────────────────
    // Área de itens começa em y=11, foco no meio: y=11 + 16 = 27
    #undef  MENU_FOCUS_Y
    #define MENU_FOCUS_Y  27
    #undef  MENU_ROW_H
    #define MENU_ROW_H    16

    ui_fill(0, MENU_FOCUS_Y, 126, MENU_ROW_H);

    // ── Setas ───────────────────────────────────────────────
    if (menu_index > 0)
    {
        // ▲ 3px acima do bloco de foco
        ui_fill(61, MENU_FOCUS_Y - 4, 6, 1);
        ui_fill(62, MENU_FOCUS_Y - 5, 4, 1);
        ui_fill(63, MENU_FOCUS_Y - 6, 2, 1);
    }
    if (menu_index < MENU_SIZE - 1)
    {
        // ▼ 3px abaixo do bloco de foco
        ui_fill(61, MENU_FOCUS_Y + MENU_ROW_H + 3, 6, 1);
        ui_fill(62, MENU_FOCUS_Y + MENU_ROW_H + 4, 4, 1);
        ui_fill(63, MENU_FOCUS_Y + MENU_ROW_H + 5, 2, 1);
    }

    // ── Itens: sel-1, sel, sel+1 ────────────────────────────
    for (int offset = -1; offset <= 1; offset++)
    {
        int idx = menu_index + offset;
        if (idx < 0 || idx >= MENU_SIZE) continue;

        // Topo da linha deste item
        int line_top = MENU_FOCUS_Y + offset * MENU_ROW_H;
        // Y do texto: centralizado na linha (fonte ~10px, linha 16px → margem 3px)
        int text_y = line_top + 3;

        if (offset == 0)
        {
            // Texto preto sobre fundo branco
            hal_display_set_color_inverted(true);
            hal_display_print_str(8, text_y, menu_items[idx]);
            hal_display_set_color_inverted(false);
        }
        else
        {
            // Itens adjacentes: XOR = cinza sobre fundo preto
            hal_display_set_dimmed(true);
            hal_display_print_str(8, text_y, menu_items[idx]);
            hal_display_set_dimmed(false);
        }
    }

    // ── Rodapé ──────────────────────────────────────────────
    ui_hline(0, 54, 128);
    hal_display_set_dimmed(true);
    ui_str(2, 56, menu_hints[menu_index]);
    hal_display_set_dimmed(false);

    // ── Scrollbar ───────────────────────────────────────────
    int sb_area = 43;  // altura da área de scroll (11 a 54)
    int thumb_h = sb_area / MENU_SIZE;
    if (thumb_h < 4) thumb_h = 4;
    int thumb_y = 11 + (menu_index * (sb_area - thumb_h)) / (MENU_SIZE - 1);

    hal_display_set_dimmed(true);
    ui_fill(126, 11, 2, sb_area);
    hal_display_set_dimmed(false);
    ui_fill(126, thumb_y, 2, thumb_h);
}

// ============================================================
// GRÁFICO
// ============================================================
static void tela_graph()
{
    hal_display_font_small();

    hal_display_set_dimmed(true);
    ui_str(GRAPH_X, 1, "TEMP C");
    hal_display_set_dimmed(false);

    // Grades e eixo Y
    for (int m = 0; m <= GRAPH_TMAX; m += 100)
    {
        int16_t py = GRAPH_Y + GRAPH_H
                     - (int16_t)((long)m * GRAPH_H / GRAPH_TMAX);
        hal_display_set_dimmed(true);
        ui_hline(GRAPH_X, py, GRAPH_W);
        hal_display_set_dimmed(false);

        if (m == 0 || m == 200 || m == 400)
        {
            hal_display_set_dimmed(true);
            ui_print_int_pad(0, py - 3, m, 3, 1);
            hal_display_set_dimmed(false);
        }
    }

    // Linha tracejada do setpoint
    int16_t spy = GRAPH_Y + GRAPH_H
                  - (int16_t)((long)(int)setpoint * GRAPH_H / GRAPH_TMAX);
    hal_display_set_dimmed(true);
    ui_dashed_hline(GRAPH_X, spy, GRAPH_W, 3, 3);
    ui_str(GRAPH_X + GRAPH_W - 10, spy - 7, "SP");
    hal_display_set_dimmed(false);

    // Curva
    for (int i = 0; i < GRAPH_W - 1; i++)
    {
        int idx1 = (g_idx + i)     % GRAPH_W;
        int idx2 = (g_idx + i + 1) % GRAPH_W;
        int16_t y1 = GRAPH_Y + GRAPH_H
                     - (int16_t)((long)g_buf[idx1] * GRAPH_H / 255);
        int16_t y2 = GRAPH_Y + GRAPH_H
                     - (int16_t)((long)g_buf[idx2] * GRAPH_H / 255);
        hal_display_draw_line(GRAPH_X + i, y1, GRAPH_X + i + 1, y2);
    }

    // Ponto atual
    int16_t cy = GRAPH_Y + GRAPH_H
                 - (int16_t)((long)g_buf[(g_idx - 1 + GRAPH_W) % GRAPH_W]
                             * GRAPH_H / 255);
    ui_rect(GRAPH_X + GRAPH_W - 3, cy - 1, 3, 3);

    // Borda da área
    hal_display_set_dimmed(true);
    ui_rect(GRAPH_X, GRAPH_Y, GRAPH_W, GRAPH_H);
    hal_display_set_dimmed(false);

    // Rodapé
    ui_hline(0, 54, 128);
    ui_print_int_pad(GRAPH_X, 56, (int)controle_get_temp(), 3, 1);
    hal_display_draw_char(GRAPH_X + 18, 56, 'C');
    hal_display_set_dimmed(true);
    ui_str(GRAPH_X + 30, 56, "SP:");
    ui_print_int_pad(GRAPH_X + 47, 56, (int)setpoint, 3, 1);
    hal_display_draw_char(GRAPH_X + 62, 56, 'C');
    hal_display_set_dimmed(false);
}

// ============================================================
// TELA STANDBY
// ============================================================
static void tela_standby()
{
    hal_display_set_dimmed(true);
    ui_str(36, 1, "STANDBY");
    ui_hline(0, 9, 128);

    // Temp ambiente esmaecida e grande
    ui_print_int_pad(2, 14, (int)controle_get_temp(), 2, 8);
    hal_display_set_font_scale(4);
    hal_display_draw_char(2 + 2 * 32 + 4, 14, 'C');
    hal_display_set_font_scale(1);

    ui_hline(0, 52, 128);
    ui_str(1, 55, "SP ---  P:----  F:OFF");
    hal_display_set_dimmed(false);
}

// ============================================================
// INPUT
// ============================================================
static void handle_input()
{
    int8_t delta = hal_encoder_get_delta();

    if (delta != 0)
        standby_timer = millis();   // qualquer giro reseta o timer de standby

    switch (state)
    {
        case UI_RUN:
            break;   // giro na RUN não altera nada — vai ao menu

        case UI_EDIT_SP:
            if (delta)
            {
                setpoint += delta * 5.0f;
                if (setpoint < 50)  setpoint = 50;
                if (setpoint > 450) setpoint = 450;
                controle_set_temp(setpoint);
            }
            break;

        case UI_MENU:
            menu_index += delta;
            if (menu_index < 0)          menu_index = 0;
            if (menu_index >= MENU_SIZE) menu_index = MENU_SIZE - 1;
            break;

        default: break;
    }

    // Clique com debounce
    if (hal_encoder_pressed() && (millis() - last_click > 200))
    {
        last_click    = millis();
        standby_timer = millis();

        switch (state)
        {
            case UI_RUN:
                state = UI_MENU;
                break;

            case UI_EDIT_SP:
                if (delta)
                {
                    setpoint += delta * 10.0f;   // era 5, agora 10 — 1 clique = 10°C
                    if (setpoint < 50)  setpoint = 50;
                    if (setpoint > 450) setpoint = 450;
                     controle_set_temp(setpoint);
                }
                break;

            case UI_MENU:
                if      (menu_index == 0) state = UI_EDIT_SP;
                else if (menu_index == 1) state = UI_GRAPH;
                else if (menu_index == 2) state = UI_RUN;    // PERFIS (futuro)
                else if (menu_index == 3) state = UI_RUN;  
                else if (menu_index == 4) state = UI_STANDBY;  // FAN (futuro)
                else                      state = UI_RUN;    // VOLTAR
                break;

            case UI_GRAPH:
                state = UI_RUN;
                break;

            case UI_STANDBY:
                state = UI_RUN;
                break;
        }
    }
}

// ============================================================
// INIT / LOOP
// ============================================================
void ui_init()
{
    hal_display_init();
    hal_encoder_init();
    standby_timer = millis();
}

void ui_update()
{
    handle_input();

    // Blink do setpoint em edição (500 ms)
    if (millis() - last_blink > 500)
    {
        last_blink = millis();
        sp_blink   = !sp_blink;
    }

    // Standby automático após 5 min sem interação
    // (descomente quando controle_standby() estiver implementado)
    // if (state == UI_RUN && millis() - standby_timer > 300000UL)
    //     state = UI_STANDBY;

    graph_push(controle_get_temp());

    hal_display_begin();
    do
    {
        switch (state)
        {
            case UI_RUN:      tela_run();      break;
            case UI_EDIT_SP:  tela_edit_sp();  break;
            case UI_MENU:     tela_menu();     break;
            case UI_GRAPH:    tela_graph();    break;
            case UI_STANDBY:  tela_standby();  break;
        }
    } while (hal_display_next());
}

