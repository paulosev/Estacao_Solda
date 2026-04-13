# 🔥 Estação de Ar Quente SMD – v4

Estação de retrabalho SMD com controle digital PID, interface OLED intuitiva e persistência de configurações.  
Compatível com **STM32F103** (Blue Pill) e **CH32V203**.

---

## ✨ Destaques da versão atual (v4)

### Interface do Usuário (UI)
- Tela principal com temperatura em **fonte gigante**, status `RUN/STOP`, setpoint e barras de potência e fan.
- **Ajuste direto do fan** girando o encoder na tela principal.
- **Edição de setpoint** com valor piscante e confirmação por clique simples.
- **Menu PID** com navegação rolante centralizada (apenas um item por vez), edição de Kp, Ki e Kd.
- **Navegação por gestos**:
  - Clique simples: confirma / avança.
  - Duplo clique: cancela / volta / acessa menu PID.
  - Clique longo: ação alternativa (ex.: confirmar e sair).
- **Persistência total** na memória flash (EEPROM emulada): setpoint, velocidade do fan e ganhos PID são salvos automaticamente ao serem alterados.

### Controle de Temperatura
- PID (QuickPID) com anti-windup e limites ajustáveis.
- **Feedforward** baseado na velocidade do fan (compensação de perda térmica).
- Modulação **Sigma‑Delta** para acionamento do TRIAC (compatível com driver MOC3443).
- **Compensação de junta fria** via NTC (leitura interna).
- **Detecção de falha do termopar**: desliga aquecimento e força fan no máximo.
- **Standby automático** ao encaixar a pistola no suporte magnético (temperatura reduzida).

---

## 🛠️ Hardware

| Componente               | Descrição                                                                 |
|--------------------------|---------------------------------------------------------------------------|
| Microcontrolador         | STM32F103C8T6 (Blue Pill) ou CH32V203C8T6                                 |
| Display                  | OLED 0.96" SSD1306 (I2C)                                                  |
| Encoder                  | KY‑040 (com capacitor de debounce 100nF)                                   |
| Amplificador termopar    | MCP6022 (ganho 151)                                                       |
| Termopar                 | Tipo K                                                                     |
| Driver TRIAC             | MOC3443 (zero‑cross interno)                                              |
| TRIAC de potência        | BTA24‑600                                                                  |
| Fan                      | 24V PWM (25 kHz, inaudível)                                                |
| Chave magnética          | Suporte da pistola (ativa standby)                                         |

### Pinagem

| Pino | Função                              |
|------|-------------------------------------|
| PA0  | TRIAC (saída digital)               |
| PA1  | FAN PWM (25 kHz)                    |
| PA2  | Termopar ADC (saída MCP6022)        |
| PA3  | NTC ADC (divisor R11/RT1)           |
| PA4  | Chave magnética (pull‑up interno)   |
| PB6  | I2C SCL (OLED)                      |
| PB7  | I2C SDA (OLED)                      |
| PB10 | Encoder CLK                         |
| PB11 | Encoder DT                          |
| PB12 | Encoder SW (botão)                  |

---

## 📁 Estrutura do Projeto
Estacao_Solda_v4/
├── include/
│ └── config.h ← Constantes e pinagem
├── lib/
│ ├── hal/
│ │ ├── hal_io.h / .cpp ← GPIO, ADC, PWM
│ │ ├── hal_display.h / .cpp ← Display OLED (U8g2)
│ │ └── hal_encoder.h / .cpp ← Encoder rotativo
│ ├── control/
│ │ ├── controle_temp.h / .cpp ← PID, Feedforward, Sigma‑Delta, persistência
│ └── app/
│ ├── app.h / .cpp ← Lógica de alto nível
│ ├── ui.h / .cpp ← Interface gráfica e navegação
└── src/
└── main.cpp

---

## ⚙️ Funcionalidades Implementadas

- [x] Tela principal com temperatura gigante e status.
- [x] Barras de potência (heater) e fan (5 níveis).
- [x] Ajuste direto do fan na tela principal.
- [x] Edição de setpoint com feedback visual.
- [x] Menu PID com navegação simplificada.
- [x] Persistência de configurações na flash (EEPROM emulada).
- [x] Standby automático via chave magnética.
- [x] Compensação da junta fria (NTC).
- [x] Detecção de termopar aberto / curto.
- [x] Controle PID com feedforward do fan.
- [x] Modulação Sigma‑Delta para o TRIAC.
- [x] PWM de alta frequência para o fan (25 kHz).
- [x] Suporte a STM32 e CH32V com o mesmo código.

---

## 🔧 Calibração

### Termopar
1. Meça a tensão na saída do MCP6022 com o termopar a uma temperatura conhecida.
2. Calcule: `TERMOPAR_COEF = Temperatura (°C) / Tensão (V)`.
3. Atualize a constante `TERMOPAR_COEF` em `config.h`.

### NTC
- A fórmula de conversão usa os parâmetros `NTC_BETA`, `NTC_R0` e `NTC_T0` definidos em `config.h`.  
  Ajuste conforme o NTC utilizado.

---

## 🚀 Compilação e Upload

| Plataforma | Board                    | Framework | Upload          |
|------------|--------------------------|-----------|-----------------|
| STM32      | bluepill_f103c8          | Arduino   | ST‑Link         |
| CH32V      | genericCH32V203C8T6      | Arduino   | WCH‑Link (wlink)|

**Bibliotecas necessárias** (definidas em `platformio.ini`):
- `QuickPID`
- `U8g2`

---

## 📌 Pendências / Melhorias Futuras

- [ ] Autotune do PID.
- [ ] Perfis de temperatura (ex.: chumbo / lead‑free).
- [ ] Curva de resfriamento automático (cooldown).
- [ ] Linearização do termopar tipo K via LUT.
- [ ] Log de dados pela Serial para ajuste fino.

---

## 📝 Histórico de Versões

### v4
- UI totalmente reformulada (temperatura gigante, barras, navegação simplificada).
- Persistência de configurações via EEPROM emulada.
- Decodificação robusta do encoder (máquina de estados).
- Detecção de cliques simples, duplo e longo.
- Correções na modulação Sigma‑Delta e no feedforward.

### v3
- Primeira versão com interface gráfica funcional (OLED + encoder).

### v2
- Controle PID, feedforward, detecção de falhas e suporte a STM32/CH32V.

---

**Desenvolvido para a comunidade de eletrônica e hobistas.**  
Contribuições e sugestões são bem‑vindas!