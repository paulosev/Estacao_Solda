#pragma once

/*
 * ============================================================
 * CONTROLE DE TEMPERATURA (PID + Sigma-Delta)
 * ============================================================
 */

// Inicializa controle
void controle_init();

// Atualiza controle (loop principal)
void controle_update();

// Define setpoint
void controle_set_temp(float temp);

// Retorna setpoint
float controle_get_setpoint();

// Retorna temperatura atual
float controle_get_temp();

// Retorna potência (0–100%)
float controle_get_power();

// Define velocidade do fan (0–100%)
void controle_set_fan(float fan);

// Retorna velocidade do fan (0–100%)
float controle_get_fan(); 

// Retorna estado standby
bool controle_is_standby();

// Define KP PID
void controle_set_kp(float sp_kp);

// Define KI PID
void controle_set_ki(float sp_ki);

// Define KD PID
void controle_set_kd(float sp_kd);

// Retorna KP PID
float controle_get_kp();

// Retorna KI PID
float controle_get_ki();

// Retorna KD PID
float controle_get_kd();