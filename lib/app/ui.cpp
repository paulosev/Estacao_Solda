#include "ui.h"
#include "hal_display.h"
#include "hal_encoder.h"
#include "controle_temp.h"
#include "config.h"
#include <Arduino.h>

// ============================================================
// ESTADOS DA UI
// ============================================================
enum UIState {
    UI_RUN,
    UI_EDIT_SP,
    UI_PID_MENU,
    UI_PID_EDIT_KP,
    UI_PID_EDIT_KI,
    UI_PID_EDIT_KD
};

static UIState state = UI_RUN;
static uint8_t pid_selection = 0;  // 0=Kp, 1=Ki, 2=Kd

// ============================================================
// VARIÁVEIS DE CONTROLE
// ============================================================
static float    setpoint = 250.0f;
static float    fan_speed = 100.0f;
static bool     blink_state = false;
static uint32_t last_blink = 0;

// ============================================================
// VARIÁVEIS PARA DETECÇÃO DE BOTÃO
// ============================================================
static bool     button_was_pressed = false;
static uint32_t button_press_time = 0;
static bool     button_hold_detected = false;
static uint32_t last_click_time = 0;
static int      pending_clicks = 0;

// ============================================================
// PROTÓTIPOS DAS FUNÇÕES
// ============================================================
static void draw_str(int x, int y, const char* s);
static void draw_int_small(int x, int y, int val, int digits);
static void draw_int_large(int x, int y, int val, int digits);
static void draw_bar(int x, int y, int w, int h, int value, int segments);
static void draw_base_layout(int big_value, const char* status, bool blink);
static void draw_run_screen();
static void draw_edit_sp_screen();
static void draw_pid_menu_screen();
static void draw_pid_edit_screen(const char* name, float value);

static void on_single_click();
static void on_double_click();
static void on_long_press();
static void update_button_state();
static void process_encoder_delta(int delta);
static void handle_input();

// ============================================================
// AUXILIARES DE DESENHO
// ============================================================
static void draw_str(int x, int y, const char* s) {
    hal_display_font_small();
    hal_display_print_str(x, y, s);
}

static void draw_int_small(int x, int y, int val, int digits) {
    hal_display_font_small();
    char buf[8];
    int v = val;
    for (int i = digits-1; i >= 0; i--) {
        buf[i] = '0' + (v % 10);
        v /= 10;
    }
    buf[digits] = '\0';
    hal_display_print_str(x, y, buf);
}

static void draw_int_large(int x, int y, int val, int digits) {
    hal_display_font_large();
    char buf[8];
    int v = val;
    for (int i = digits-1; i >= 0; i--) {
        buf[i] = '0' + (v % 10);
        v /= 10;
    }
    buf[digits] = '\0';
    hal_display_print_str(x, y, buf);
}

static void draw_bar(int x, int y, int w, int h, int value, int segments) {
    int seg_w = (w - (segments-1)) / segments;
    int filled = (value * segments) / 100;
    for (int i = 0; i < segments; i++) {
        int bx = x + i*(seg_w+1);
        if (i < filled)
            hal_display_fill_rect(bx, y, seg_w, h);
        else
            hal_display_draw_rect(bx, y, seg_w, h);
    }
}

// ============================================================
// DESENHO DAS TELAS
// ============================================================
static void draw_base_layout(int big_value, const char* status, bool blink) {
    // ==================== METADE SUPERIOR ====================
    if (!blink || blink_state) {
        hal_display_set_font_scale(8);
        char buf[4];
        int v = big_value;
        for (int i = 2; i >= 0; i--) {
            buf[i] = '0' + (v % 10);
            v /= 10;
        }
        buf[3] = '\0';
        hal_display_print_str(2, 8, buf);
    }

    hal_display_set_font_scale(4);
    hal_display_draw_char(100, 2, '\xB0');
    hal_display_draw_char(112, 2, 'C');

    hal_display_font_small();
    int status_w = strlen(status) * 6;
    draw_str(128 - status_w - 2, 44, status);

    hal_display_draw_hline(0, 52, 128);

    // ==================== RODAPÉ ====================
    draw_int_small(1, 55, (int)setpoint, 3);
    hal_display_draw_char(18, 55, '\xB0');
    hal_display_draw_char(24, 55, 'C');

    hal_display_set_dimmed(true);
    hal_display_draw_vline(35, 53, 11);
    hal_display_set_dimmed(false);

    hal_display_draw_char(38, 55, 'P');
    draw_bar(45, 55, 38, 7, (int)controle_get_power(), 5);

    hal_display_set_dimmed(true);
    hal_display_draw_vline(88, 53, 11);
    hal_display_set_dimmed(false);

    hal_display_draw_char(91, 55, 'F');
    draw_bar(98, 55, 29, 7, (int)fan_speed, 5);

    hal_display_set_font_scale(1);
}

static void draw_run_screen() {
    int temp = (int)controle_get_temp();
    const char* status = controle_is_standby() ? "STOP" : "RUN";
    draw_base_layout(temp, status, false);
}

static void draw_edit_sp_screen() {
    draw_base_layout((int)setpoint, "SET", true);
}

static void draw_pid_menu_screen() {
    draw_str(5, 10, "AJUSTAR PID");
    hal_display_draw_hline(0, 20, 128);

    const char* names[] = {"Kp", "Ki", "Kd"};

    // Usar escala 4 (profont22) que contém letras
    hal_display_set_font_scale(4);
    int name_w = strlen(names[pid_selection]) * 11; // largura aprox. por caractere
    int name_x = (128 - name_w) / 2;
    hal_display_print_str(name_x, 34, names[pid_selection]);

    hal_display_font_small();
    draw_str(10, 62, "Gira:muda  Clique:edita  2x:sai");
}

static void draw_pid_edit_screen(const char* name, float value) {
    draw_str(10, 10, "EDITAR ");
    draw_str(50, 10, name);
    hal_display_draw_hline(0, 20, 128);

    hal_display_font_large();
    int x = (128 - 70)/2;
    char buf[10];
    dtostrf(value, 5, 2, buf);
    hal_display_print_str(x, 30, buf);
    hal_display_font_small();

    draw_str(10, 62, "Gira:ajusta  Clique:conf.  2x:cancela");
}

// ============================================================
// AÇÕES DOS CLIQUES
// ============================================================
static void on_single_click() {
    switch (state) {
        case UI_RUN:
            state = UI_EDIT_SP;
            break;
        case UI_EDIT_SP:
            controle_set_temp(setpoint);
            state = UI_RUN;
            break;
        case UI_PID_MENU:
            if (pid_selection == 0) state = UI_PID_EDIT_KP;
            else if (pid_selection == 1) state = UI_PID_EDIT_KI;
            else state = UI_PID_EDIT_KD;
            break;
        case UI_PID_EDIT_KP:
        case UI_PID_EDIT_KI:
        case UI_PID_EDIT_KD:
            state = UI_PID_MENU;
            break;
    }
}

static void on_double_click() {
    switch (state) {
        case UI_RUN:
            state = UI_PID_MENU;
            break;
        case UI_EDIT_SP:
            state = UI_RUN;
            break;
        case UI_PID_MENU:
            state = UI_RUN;
            break;
        case UI_PID_EDIT_KP:
        case UI_PID_EDIT_KI:
        case UI_PID_EDIT_KD:
            state = UI_PID_MENU;
            break;
    }
}

static void on_long_press() {
    switch (state) {
        case UI_RUN:
            break;
        case UI_EDIT_SP:
            controle_set_temp(setpoint);
            state = UI_RUN;
            break;
        case UI_PID_MENU:
            state = UI_RUN;
            break;
        case UI_PID_EDIT_KP:
        case UI_PID_EDIT_KI:
        case UI_PID_EDIT_KD:
            state = UI_PID_MENU;
            break;
    }
}

// ============================================================
// ATUALIZAÇÃO DO ESTADO DO BOTÃO (usa HAL)
// ============================================================
static void update_button_state() {
    bool button_is_pressed = hal_encoder_button_is_pressed();

    if (button_is_pressed && !button_was_pressed) {
        button_press_time = millis();
        button_hold_detected = false;
    }
    else if (!button_is_pressed && button_was_pressed) {
        uint32_t duration = millis() - button_press_time;
        
        if (duration < 500) {
            uint32_t now = millis();
            if (now - last_click_time < 300) {
                pending_clicks = 0;
                on_double_click();
            } else {
                pending_clicks = 1;
                last_click_time = now;
            }
        } else {
            on_long_press();
        }
    }
    else if (button_is_pressed && button_was_pressed) {
        if (!button_hold_detected && (millis() - button_press_time >= 500)) {
            button_hold_detected = true;
        }
    }

    button_was_pressed = button_is_pressed;

    if (pending_clicks == 1 && (millis() - last_click_time > 300)) {
        pending_clicks = 0;
        on_single_click();
    }
}

// ============================================================
// PROCESSAMENTO DO ENCODER
// ============================================================
static void process_encoder_delta(int delta) {
    switch (state) {
        case UI_RUN:
            fan_speed += delta * 5.0f;
            if (fan_speed < 20) fan_speed = 20;
            if (fan_speed > 100) fan_speed = 100;
            controle_set_fan(fan_speed);
            break;

        case UI_EDIT_SP:
            setpoint += delta * 5.0f;
            if (setpoint < 50) setpoint = 50;
            if (setpoint > 450) setpoint = 450;
            break;

        case UI_PID_MENU:
            {
                int new_sel = (int)pid_selection + delta;
                new_sel = new_sel % 3;
                if (new_sel < 0) new_sel += 3;
                pid_selection = (uint8_t)new_sel;
            }
            break;

        case UI_PID_EDIT_KP: {
            float kp = controle_get_kp() + delta * 0.1f;
            if (kp < 0.0f) kp = 0.0f;
            controle_set_kp(kp);
            break;
        }
        case UI_PID_EDIT_KI: {
            float ki = controle_get_ki() + delta * 0.01f;
            if (ki < 0.0f) ki = 0.0f;
            controle_set_ki(ki);
            break;
        }
        case UI_PID_EDIT_KD: {
            float kd = controle_get_kd() + delta * 0.01f;
            if (kd < 0.0f) kd = 0.0f;
            controle_set_kd(kd);
            break;
        }
    }
}

static void handle_input() {
    update_button_state();

    int delta = hal_encoder_get_delta();
    if (delta != 0) {
        process_encoder_delta(delta);
    }

    if (millis() - last_blink > 500) {
        last_blink = millis();
        blink_state = !blink_state;
    }
}

// ============================================================
// INIT / LOOP
// ============================================================
void ui_init() {
    hal_display_init();
    hal_encoder_init();
    setpoint = controle_get_setpoint();
    fan_speed = controle_get_fan();
    state = UI_RUN;
    button_was_pressed = hal_encoder_button_is_pressed();
}

void ui_update() {
    handle_input();

    hal_display_begin();
    do {
        switch (state) {
            case UI_RUN:
                draw_run_screen();
                break;
            case UI_EDIT_SP:
                draw_edit_sp_screen();
                break;
            case UI_PID_MENU:
                draw_pid_menu_screen();
                break;
            case UI_PID_EDIT_KP:
                draw_pid_edit_screen("Kp", controle_get_kp());
                break;
            case UI_PID_EDIT_KI:
                draw_pid_edit_screen("Ki", controle_get_ki());
                break;
            case UI_PID_EDIT_KD:
                draw_pid_edit_screen("Kd", controle_get_kd());
                break;
        }
    } while (hal_display_next());
}