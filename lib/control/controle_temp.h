#pragma once

/*
 * Camada de controle térmico
 */

// Códigos de erro do sensor
#define SENSOR_OK          0
#define SENSOR_ABERTO     -1    // termopar desconectado (ADC > 3900)
#define SENSOR_CURTO      -2    // curto ou inversão (ADC < 5)

// Inicializa PID e variáveis internas
void controle_init();

// Executa um ciclo de controle (chamar a cada CONTROLE_PERIODO_MS)
void controle_update();

// Define temperatura alvo (°C). Use 0 para desligar.
void controle_set_temp(float temp);

// Retorna temperatura atual (°C)
float controle_get_temp();

// Retorna último status do sensor (SENSOR_OK, SENSOR_ABERTO, SENSOR_CURTO)
int controle_get_sensor_status();
