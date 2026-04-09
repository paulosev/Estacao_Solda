#include "app.h"
#include "controle_temp.h"
#include "hal_io.h"

/*
 * ================================================================
 * CAMADA DE APLICAÇÃO
 * ================================================================
 *
 * Responsável pela lógica de alto nível:
 *  - Leitura da chave magnética (pistola no suporte ou em uso)
 *  - Definição do setpoint
 *  - Acionamento/desligamento seguro do sistema
 */

// Temperatura alvo de operação (°C) — ajustar conforme o processo
static const float SETPOINT_OPERACAO = 350.0f;

static bool sistema_ligado = false;


/*
 * Inicialização da aplicação
 */
void app_init()
{
    // Incializa controle de temperatura
    controle_init();

    // Inicializa interface gráfica
    ui_init();
}


/*
 * Loop principal da aplicação
 * Deve ser chamado sem delay no loop() do Arduino.
 */
void app_update()
{
    bool chave = hal_chave_ligada();

    // Detecta transição desligado → ligado
    if (chave && !sistema_ligado)
    {
        sistema_ligado = true;
        controle_set_temp(SETPOINT_OPERACAO);
    }

    // Detecta transição ligado → desligado
    if (!chave && sistema_ligado)
    {
        sistema_ligado = false;

        // Desliga aquecimento e FAN imediatamente
        controle_set_temp(0.0f);
        hal_triac_write(false);
        hal_fan_write(0);
    }

    if (sistema_ligado)
    {
        // Executa ciclo de controle térmico
        controle_update();
    }

    // Atualiza interface do usuário
    ui_update();
}
