#include "hal_encoder.h"
#include <Arduino.h>

/*
 * ============================================================
 * CONFIGURAÇÃO DOS PINOS
 * ============================================================
 *
 * Ideal mover para config.h depois
 */
#define ENC_A   PA6
#define ENC_B   PA7
#define ENC_BTN PA5

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
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_BTN, INPUT_PULLUP);

    // Estado inicial
    last_state = digitalRead(ENC_A);
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
    int current = digitalRead(ENC_A);

    // Detecta mudança de estado
    if (current != last_state)
    {
        // Determina direção
        if (digitalRead(ENC_B) != current)
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
    return digitalRead(ENC_BTN) == LOW;
}