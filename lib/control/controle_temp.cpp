#include "controle_temp.h"
#include "hal_io.h"
#include <QuickPID.h>
#include <Arduino.h>
#include <EEPROM.h>

// ============================================================
// ENDEREÇOS NA EEPROM EMULADA
// ============================================================
#define EE_ADDR_SETPOINT    0
#define EE_ADDR_FAN         4
#define EE_ADDR_KP          8
#define EE_ADDR_KI         12
#define EE_ADDR_KD         16
#define EE_MAGIC           20

#define EE_MAGIC_VALUE     0xA5A5

// ============================================================
// VARIÁVEIS
// ============================================================
static float temp = 25.0f;
static float temp_filtrada = 25.0f;
static float setpoint = 200.0f;
static float power = 0.0f;
static float kp = 2.0f;
static float ki = 0.5f;
static float kd = 1.0f;

// PID
static float input, output, set;
QuickPID pid(&input, &output, &set, kp, ki, kd, QuickPID::Action::direct);

// Sigma-delta
static float acumulador = 0.0f;

// Fan (0–100%)
static float fan = 100.0f;

// Standby
static bool standby = false;

// ============================================================
// FUNÇÕES INTERNAS DE PERSISTÊNCIA
// ============================================================
static void load_settings() {
    uint16_t magic;
    EEPROM.get(EE_MAGIC, magic);
    if (magic != EE_MAGIC_VALUE) {
        // Primeira execução: mantém os valores padrão já definidos
        return;
    }

    float val;
    EEPROM.get(EE_ADDR_SETPOINT, val);
    if (!isnan(val) && val >= 50.0f && val <= 450.0f) setpoint = val;

    EEPROM.get(EE_ADDR_FAN, val);
    if (!isnan(val) && val >= 0.0f && val <= 100.0f) fan = val;

    EEPROM.get(EE_ADDR_KP, val);
    if (!isnan(val) && val >= 0.0f) kp = val;

    EEPROM.get(EE_ADDR_KI, val);
    if (!isnan(val) && val >= 0.0f) ki = val;

    EEPROM.get(EE_ADDR_KD, val);
    if (!isnan(val) && val >= 0.0f) kd = val;

    // Atualiza os parâmetros do PID com os valores carregados
    pid.SetTunings(kp, ki, kd);
}

static void save_all_settings() {
    EEPROM.put(EE_ADDR_SETPOINT, setpoint);
    EEPROM.put(EE_ADDR_FAN, fan);
    EEPROM.put(EE_ADDR_KP, kp);
    EEPROM.put(EE_ADDR_KI, ki);
    EEPROM.put(EE_ADDR_KD, kd);
    EEPROM.put(EE_MAGIC, (uint16_t)EE_MAGIC_VALUE);
    // Em STM32 e CH32, o commit é automático.
}

// ============================================================
// CONVERSÕES ADC → TEMPERATURA
// ============================================================
static float adc_to_temp(uint16_t adc) {
    float voltage = (adc / 4095.0f) * 3.3f;
    // ganho do amp (~151) – calibração pendente
    float temp = voltage * 100.0f;
    return temp;
}

static float ntc_to_temp(uint16_t adc) {
    // placeholder simples
    float voltage = (adc / 4095.0f) * 3.3f;
    float temp = voltage * 50.0f;
    return temp;
}

// ============================================================
// FILTRO EXPONENCIAL
// ============================================================
static float filtro(float in) {
    const float alpha = 0.1f;
    temp_filtrada += alpha * (in - temp_filtrada);
    return temp_filtrada;
}

// ============================================================
// INIT
// ============================================================
void controle_init() {
    load_settings();   // Carrega configurações salvas na flash
    pid.SetMode(QuickPID::Control::automatic);
    pid.SetOutputLimits(0.0f, 100.0f);
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void controle_update() {
    // Leitura ADC
    uint16_t adc_tc  = hal_adc_termopar();
    uint16_t adc_ntc = hal_adc_ntc();

    float temp_tc  = adc_to_temp(adc_tc);
    float temp_ntc = ntc_to_temp(adc_ntc);

    // Compensação de junta fria
    temp = temp_tc + temp_ntc;

    // Filtro
    input = filtro(temp);

    // Standby (dock magnético)
    if (!hal_chave_ligada()) {
        standby = true;
        set = 50.0f;   // temperatura segura
    } else {
        standby = false;
        set = setpoint;
    }

    // PID
    pid.Compute();
    power = output;

    // Compensação por fan
    float power_comp = power + (fan * 0.2f);
    if (power_comp > 100.0f) power_comp = 100.0f;

    // Sigma-delta
    acumulador += power_comp;
    if (acumulador >= 100.0f) {
        hal_triac_write(true);
        acumulador -= 100.0f;
    } else {
        hal_triac_write(false);
    }

    // Controle do fan
    if (input > 300.0f && fan < 50.0f)
        fan = 50.0f;
    if (input < 70.0f)
        fan = 0.0f;

    uint16_t fan_pwm = (uint16_t)(fan * 10.0f);  // 0..100% → 0..1000 permil
    hal_fan_write(fan_pwm);
}

// ============================================================
// GET/SET (com persistência automática)
// ============================================================
void controle_set_temp(float t) {
    if (setpoint != t) {
        setpoint = t;
        save_all_settings();
    }
}

float controle_get_setpoint() {
    return setpoint;
}

float controle_get_temp() {
    return input;
}

float controle_get_power() {
    return power;
}

void controle_set_fan(float f) {
    if (fan != f) {
        fan = f;
        save_all_settings();
    }
}

float controle_get_fan() {
    return fan;
}

bool controle_is_standby() {
    return standby;
}

void controle_set_kp(float sp_kp) {
    if (kp != sp_kp) {
        kp = sp_kp;
        pid.SetTunings(kp, ki, kd);
        save_all_settings();
    }
}

void controle_set_ki(float sp_ki) {
    if (ki != sp_ki) {
        ki = sp_ki;
        pid.SetTunings(kp, ki, kd);
        save_all_settings();
    }
}

void controle_set_kd(float sp_kd) {
    if (kd != sp_kd) {
        kd = sp_kd;
        pid.SetTunings(kp, ki, kd);
        save_all_settings();
    }
}

float controle_get_kp() { return kp; }
float controle_get_ki() { return ki; }
float controle_get_kd() { return kd; }