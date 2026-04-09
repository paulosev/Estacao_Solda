#include "controle_temp.h"
#include "hal_io.h"
#include "config.h"
#include <QuickPID.h>
#include <math.h>

/*
 * ================================================================
 * VARIÁVEIS INTERNAS
 * ================================================================
 */

static float temperatura_atual = 25.0f;
static float setpoint           = 0.0f;
static float entrada_pid;
static float saida_pid;
static int   sensor_status = SENSOR_OK;

/*
 * Instância do PID
 * Saída limitada dinamicamente para deixar espaço ao feedforward.
 */
static QuickPID pid(&entrada_pid, &saida_pid, &setpoint,
                    PID_KP, PID_KI, PID_KD,
                    QuickPID::Action::direct);

/*
 * Acumulador sigma-delta
 */
static float acumulador = 0.0f;


/*
 * ================================================================
 * LEITURA DO NTC — Compensação de junta fria
 * ================================================================
 *
 * Divisor: +3.3V → R11 (10k) → nó → RT1 NTC → GND
 * NTC no lado baixo → r = R_pullup * v / (VREF - v)
 */
static float ler_ntc()
{
    uint16_t adc = hal_adc_ntc();

    float v = (adc / ADC_MAX) * VREF;

    // Proteção contra divisão por zero nas bordas da faixa ADC
    if (v < 0.01f) v = 0.01f;
    if (v > VREF - 0.01f) v = VREF - 0.01f;

    // Resistência do NTC — NTC no lado baixo do divisor
    float r = NTC_PULLUP * v / (VREF - v);

    // Equação Beta → temperatura em Kelvin
    float tempK = 1.0f / ((1.0f / NTC_T0) +
                          (1.0f / NTC_BETA) * logf(r / NTC_R0));

    return tempK - 273.15f;
}


/*
 * ================================================================
 * LEITURA DO TERMOPAR — com detecção de falha
 * ================================================================
 *
 * Retorna temperatura diferencial (°C) ou valor negativo em caso de falha:
 *   SENSOR_ABERTO : ADC > TERMOPAR_ADC_MAX (pull-up R10 puxou para 3.3V)
 *   SENSOR_CURTO  : ADC < TERMOPAR_ADC_MIN
 */
static float ler_termopar()
{
    uint16_t adc = hal_adc_termopar();

    if (adc > TERMOPAR_ADC_MAX)
    {
        sensor_status = SENSOR_ABERTO;
        return (float)SENSOR_ABERTO;
    }

    if (adc < TERMOPAR_ADC_MIN)
    {
        sensor_status = SENSOR_CURTO;
        return (float)SENSOR_CURTO;
    }

    sensor_status = SENSOR_OK;

    float v = (adc / ADC_MAX) * VREF;

    // V_out = T_diff * 41µV/°C * ganho 151
    // T_diff = V_out / (41e-6 * 151) = V_out * TERMOPAR_COEF
    return v * TERMOPAR_COEF;
}


/*
 * ================================================================
 * TEMPERATURA COMPENSADA
 * ================================================================
 *
 * Temperatura real = diferencial do termopar + temperatura ambiente (NTC)
 * Retorna valor negativo se sensor com falha.
 */
static float calcular_temperatura()
{
    float temp_tc = ler_termopar();

    if (temp_tc < 0.0f)
        return temp_tc;   // propaga código de erro

    float temp_amb = ler_ntc();

    return temp_tc + temp_amb;
}


/*
 * ================================================================
 * FEEDFORWARD
 * ================================================================
 *
 * Estima potência necessária com base no setpoint e no fluxo de ar,
 * antes do PID atuar. Reduz overshoot e melhora resposta dinâmica.
 *
 * fan_duty: 0–1000 (permil)
 * Retorna: contribuição em % de potência (0–FF_MAX)
 */
static float calcular_feedforward(float sp, uint16_t fan_duty)
{
    float fan_norm = fan_duty / 1000.0f;   // normaliza para 0.0–1.0

    float ff = (sp * FF_K_TEMP) + (fan_norm * FF_K_FAN * 100.0f);

    // Limita para não sufocar o PID
    if (ff > FF_MAX) ff = FF_MAX;
    if (ff < 0.0f)   ff = 0.0f;

    return ff;
}


/*
 * ================================================================
 * INICIALIZAÇÃO
 * ================================================================
 */
void controle_init()
{
    pid.SetMode(QuickPID::Control::automatic);

    // Anti-windup: limita integral quando a saída satura
    pid.SetAntiWindupMode(QuickPID::iAwMode::iAwClamp);

    // Saída inicial restrita — feedforward completará o restante
    pid.SetOutputLimits(0, 100.0f - FF_MAX);
}


/*
 * ================================================================
 * LOOP DE CONTROLE  (chamar a cada CONTROLE_PERIODO_MS)
 * ================================================================
 */
void controle_update()
{
    // --- Timing determinístico ---
    static uint32_t ultimo_ms = 0;
    uint32_t agora = millis();
    if (agora - ultimo_ms < (uint32_t)CONTROLE_PERIODO_MS)
        return;
    ultimo_ms = agora;


    // ============================================================
    // 1. LEITURA DE TEMPERATURA
    // ============================================================

    float t = calcular_temperatura();

    if (t < 0.0f)
    {
        // Falha de sensor — desliga aquecimento imediatamente
        hal_triac_write(false);
        hal_fan_write(1000);    // FAN no máximo para resfriar
        acumulador = 0.0f;
        return;
    }

    temperatura_atual = t;
    entrada_pid       = t;


    // ============================================================
    // 2. CONTROLE DO FAN (proporcional à temperatura)
    // ============================================================

    uint16_t fan = 0;

    if (temperatura_atual < TEMP_MIN_FAN_STOP)
    {
        // Frio → pode desligar o FAN
        fan = 0;
    }
    else
    {
        // Proporcional: 70°C → 20%, 400°C → 100%
        fan = (uint16_t)map((long)temperatura_atual, 70, 400, 200, 1000);
        fan = constrain(fan, 200, 1000);

        // Acima de 300°C o FAN nunca fica abaixo de 50%
        if (temperatura_atual > TEMP_MIN_FAN_SAFE && fan < 500)
            fan = 500;
    }

    hal_fan_write(fan);


    // ============================================================
    // 3. CÁLCULO DE POTÊNCIA (Feedforward + PID)
    // ============================================================

    float ff = calcular_feedforward(setpoint, fan);

    // Reserva espaço no PID para o feedforward não saturar a saída total
    pid.SetOutputLimits(0, 100.0f - ff);

    pid.Compute();

    float saida_total = saida_pid + ff;

    // Saturação final
    if (saida_total > 100.0f) saida_total = 100.0f;
    if (saida_total <   0.0f) saida_total = 0.0f;


    // ============================================================
    // 4. SIGMA-DELTA → sinal ON/OFF para o TRIAC
    // ============================================================
    //
    // Distribui a energia ao longo do tempo de forma uniforme.
    // O MOC3443 garante acionamento sempre no zero-cross da rede.

    acumulador += saida_total;

    // Evita acúmulo negativo (sistema frio por muito tempo)
    if (acumulador < 0.0f) acumulador = 0.0f;

    bool liga = false;

    if (acumulador >= SIGMA_LIMITE)
    {
        liga = true;
        acumulador -= SIGMA_LIMITE;
    }

    hal_triac_write(liga);
}


/*
 * ================================================================
 * API PÚBLICA
 * ================================================================
 */

void controle_set_temp(float temp)
{
    setpoint = (temp < 0.0f) ? 0.0f : temp;
}

float controle_get_temp()
{
    return temperatura_atual;
}

int controle_get_sensor_status()
{
    return sensor_status;
}
