#pragma once

/*
 * ================= CONFIGURAÇÃO DE HARDWARE =================
 */

// Pino que controla o TRIAC (via MOC3443)
#define PIN_TRIAC        PA0

// PWM do FAN (alta frequência)
#define PIN_FAN_PWM      PA1

// Entrada ADC do termopar (via amplificador MCP6022, Ganho 151)
#define PIN_TERMOPAR     PA2

// Entrada ADC do NTC (temperatura ambiente, divisor R11/RT1)
#define PIN_NTC          PA3

// Chave magnética (pull-up interno)
#define PIN_CHAVE        PA4

// Display I2C
#define PIN_I2C_SCL      PB6
#define PIN_I2C_SDA      PB7

// Encoder rotativo (KY-040)

#define PIN_ENC_CLK      PB10 // CLK (A)
#define PIN_ENC_DT       PB11 // DT (B)
#define PIN_ENC_SW       PB12 // SW (botão)


/*
 * ================= PARÂMETROS DE CONTROLE =================
 */

// Temperatura abaixo da qual o fan pode parar
#define TEMP_MIN_FAN_STOP    70.0f

// Acima disso o fan nunca pode ficar lento (segurança)
#define TEMP_MIN_FAN_SAFE    300.0f

// Período do loop de controle em ms (10 Hz)
#define CONTROLE_PERIODO_MS  100


/*
 * ================= PID =================
 * Ajustar via autotune idealmente
 */

#define PID_KP  2.0f
#define PID_KI  0.4f
#define PID_KD  0.8f


/*
 * ================= ADC =================
 */

#define ADC_MAX        4095.0f   // 12 bits
#define VREF           3.3f


/*
 * ================= TERMOPAR =================
 * Amplificador MCP6022, ganho = 1 + R9/R8 = 1 + 150k/1k = 151
 *
 * Termopar tipo K: ~41 uV/°C
 * V_out = T * 41e-6 * 151 = T * 6.191e-3  V/°C
 * TERMOPAR_COEF = 1 / 6.191e-3 = 161.5 °C/V (na saída do amp)
 *
 * Ajustar empiricamente com ponto de referência conhecido!
 */

#define TERMOPAR_COEF       161.5f       // °C/V na saída do amp (calibrar!)

// Limites ADC para detecção de falha do termopar
// > 3900 → entrada puxada para 3.3V pelo pull-up R10 (termopar aberto)
// < 5    → curto ou inversão de polaridade
#define TERMOPAR_ADC_MAX    3900
#define TERMOPAR_ADC_MIN    5


/*
 * ================= NTC =================
 * Divisor: +3.3V → R11 (10k) → nó T-AMB → RT1 NTC (10k) → GND
 * NTC no lado baixo do divisor → formula: r = R_pullup * v / (VREF - v)
 */

#define NTC_BETA       3950.0f
#define NTC_R0         10000.0f
#define NTC_T0         298.15f   // 25°C em Kelvin
#define NTC_PULLUP     10000.0f  // R11 = 10k


/*
 * ================= SIGMA-DELTA =================
 */

#define SIGMA_LIMITE   100.0f


/*
 * ================= FEEDFORWARD =================
 * ff = (setpoint * FF_K_TEMP) + (fan_norm * FF_K_FAN * 100)
 * fan_norm = fan_duty / 1000.0  (0.0 a 1.0)
 *
 * Ajustar empiricamente.
 */

#define FF_K_TEMP      0.05f    // ex: 300°C → contribui 15%
#define FF_K_FAN       0.20f    // fan full → contribui 20%
#define FF_MAX         40.0f    // teto do feedforward (%)
