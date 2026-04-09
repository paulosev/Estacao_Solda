#include "hal_encoder.h"
#include <Arduino.h>
#include "config.h"

/*
 * Armazena último estado do canal A
 * usado para detectar transição
 */
static int last_state = 0;


/*
 * ============================================================
 * INICIALIZAÇÃO
 * ============================================================
 */
void hal_encoder_init()
{
    // Entradas com pull-up interno
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT, INPUT_PULLUP);
    pinMode(PIN_ENC_SW, INPUT_PULLUP);

    // Estado inicial
    last_state = digitalRead(PIN_ENC_CLK);
}


/*
 * ============================================================
 * LEITURA DO ENCODER
 * ============================================================
 *
 * Método simples:
 * - Detecta borda no canal A
 * - Usa canal B para direção
 */
int8_t hal_encoder_get_delta()
{
    int current = digitalRead(PIN_ENC_CLK);

    // Detecta mudança de estado
    if (current != last_state)
    {
        // Determina direção
        if (digitalRead(PIN_ENC_DT) != current)
        {
            last_state = current;
            return +1; // sentido horário
        }
        else
        {
            last_state = current;
            return -1; // sentido anti-horário
        }
    }

    return 0;
}


/*
 * ============================================================
 * BOTÃO DO ENCODER
 * ============================================================
 *
 * LOW = pressionado (pull-up)
 */
bool hal_encoder_pressed()
{
    return digitalRead(PIN_ENC_SW) == LOW;
}