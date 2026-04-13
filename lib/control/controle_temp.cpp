#include "controle_temp.h"
#include "hal_io.h"
#include <QuickPID.h>
#include <Arduino.h>

/*
 * ============================================================
 * VARIÁVEIS
 * ============================================================
 */

static float temp = 25;
static float temp_filtrada = 25;
static float setpoint = 200;
static float power = 0;

// PID
static float input, output, set;
QuickPID pid(&input, &output, &set, 2.0, 0.5, 1.0, QuickPID::Action::direct);

// Sigma-delta
static float acumulador = 0;

// Fan (0–100%)
static float fan = 100;

// Standby
static bool standby = false;

/*
 * ============================================================
 * CONVERSÕES ADC → TEMPERATURA
 * (placeholder — depois calibrar!)
 * ============================================================
 */
static float adc_to_temp(uint16_t adc)
{
    float voltage = (adc / 4095.0f) * 3.3f;

    // ganho do amp (~151)
    float temp = voltage * 100.0f;

    return temp;
}

/*
 * ============================================================
 * NTC → temperatura ambiente (junta fria)
 * ============================================================
 */
static float ntc_to_temp(uint16_t adc)
{
    // placeholder simples
    float voltage = (adc / 4095.0f) * 3.3f;

    float temp = voltage * 50.0f;

    return temp;
}

/*
 * ============================================================
 * FILTRO EXPONENCIAL
 * ============================================================
 */
static float filtro(float in)
{
    const float alpha = 0.1;
    temp_filtrada += alpha * (in - temp_filtrada);
    return temp_filtrada;
}

/*
 * ============================================================
 * INIT
 * ============================================================
 */
void controle_init()
{
    pid.SetMode(QuickPID::Control::automatic);
    pid.SetOutputLimits(0, 100);
}

/*
 * ============================================================
 * LOOP PRINCIPAL
 * ============================================================
 */
void controle_update()
{
    /*
     * ----------- LEITURA ADC -----------
     */
    uint16_t adc_tc  = hal_adc_termopar();
    uint16_t adc_ntc = hal_adc_ntc();

    float temp_tc  = adc_to_temp(adc_tc);
    float temp_ntc = ntc_to_temp(adc_ntc);

    /*
     * ----------- COMPENSAÇÃO JUNTA FRIA -----------
     */
    temp = temp_tc + temp_ntc;

    /*
     * ----------- FILTRO -----------
     */
    input = filtro(temp);

    /*
     * ----------- STANDBY (dock magnético) -----------
     */
    if (!hal_chave_ligada())
    {
        standby = true;
        set = 50; // temperatura segura
    }
    else
    {
        standby = false;
        set = setpoint;
    }

    /*
     * ----------- PID -----------
     */
    pid.Compute();
    power = output;

    /*
     * ----------- COMPENSAÇÃO POR FAN -----------
     */
    float power_comp = power + (fan * 0.2f);
    if (power_comp > 100) power_comp = 100;

    /*
     * ----------- SIGMA-DELTA -----------
     */
    acumulador += power_comp;

    if (acumulador >= 100)
    {
        hal_triac_write(true);
        acumulador -= 100;
    }
    else
    {
        hal_triac_write(false);
    }

    /*
     * ----------- CONTROLE FAN -----------
     */

    // Limite mínimo em alta temperatura
    if (input > 300 && fan < 50)
        fan = 50;

    // Desliga fan frio
    if (input < 70)
        fan = 0;

    // Converte 0–100% → 0–1000 (permil)
    uint16_t fan_pwm = (fan * 10.0f);

    hal_fan_write(fan_pwm);
}

/*
 * ============================================================
 * GET/SET
 * ============================================================
 */
void controle_set_temp(float t)
{
    setpoint = t;
}

float controle_get_temp()
{
    return input;
}

float controle_get_power()
{
    return power;
}

void controle_set_fan(float f)
{
    fan = f;
}

float controle_get_fan()
{
    return fan;
}

bool controle_is_standby()
{
    return standby;
}