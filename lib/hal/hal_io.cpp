#include "hal_io.h"
#include "config.h"

/*
 * Inicialização de hardware
 */
void hal_io_init()
{
    pinMode(PIN_TRIAC,   OUTPUT);
    digitalWrite(PIN_TRIAC, LOW);   // garantia: TRIAC desligado no boot

    pinMode(PIN_FAN_PWM, OUTPUT);
    analogWrite(PIN_FAN_PWM, 0);    // FAN parado no boot

    // Chave com pull-up interno
    pinMode(PIN_CHAVE, INPUT_PULLUP);

    // ADC com resolução de 12 bits
    analogReadResolution(12);

    // PWM do fan em alta frequência (evita ruído audível)
    // STM32: analogWriteFrequency disponível via STM32duino
    // CH32V: verificar suporte do core
    analogWriteFrequency(25000);

    // PWM também em 12 bits para máxima resolução
    analogWriteResolution(12);
}

/*
 * Liga/desliga o TRIAC
 * O MOC3443 possui zero-cross interno — não é necessário ISR.
 */
void hal_triac_write(bool estado)
{
    digitalWrite(PIN_TRIAC, estado ? HIGH : LOW);
}

/*
 * Controla o FAN
 * duty: 0–1000 (permil) → mapeado para 0–4095 (12 bits PWM)
 */
void hal_fan_write(uint16_t duty)
{
    duty = constrain(duty, 0, 1000);
    uint16_t pwm = map(duty, 0, 1000, 0, 4095);
    analogWrite(PIN_FAN_PWM, pwm);
}

/*
 * Leitura do termopar (saída do MCP6022)
 */
uint16_t hal_adc_termopar()
{
    return analogRead(PIN_TERMOPAR);
}

/*
 * Leitura do NTC (divisor R11/RT1)
 */
uint16_t hal_adc_ntc()
{
    return analogRead(PIN_NTC);
}

/*
 * Chave magnética:
 * LOW  = pistola encaixada no suporte → sistema ligado
 * HIGH = pistola em uso → desligado (segurança)
 *
 * Lógica invertida pois usa pull-up interno.
 */
bool hal_chave_ligada()
{
    return digitalRead(PIN_CHAVE) == LOW;
}
