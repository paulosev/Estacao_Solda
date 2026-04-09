# Estação de Ar Quente SMD — v4
UI: barra de potência (sigma-delta) destaque quando editando animação leve 
Controle: compensação por airflow (fan) filtro melhor no ADC calibração do termopar 
Produto: standby automático perfis de temperatura proteção térmica

# Estação de Ar Quente SMD — v3

Interface gráfica para display oled 0,96" SSD1336, encoder ky-040 e u8g2 page

# Estação de Ar Quente SMD — v2

Projeto de estação de retrabalho SMD com controle digital avançado.
Microcontrolador: **STM32F103** (Blue Pill) ou **CH32V203**.

---

## Mudanças da v2

### Correções de bugs

| Arquivo | Problema | Correção |
|---|---|---|
| `controle_temp.cpp` | Fórmula NTC errada (NTC no lado alto) | Corrigida para NTC no lado baixo: `r = R_pullup * v / (VREF - v)` |
| `controle_temp.cpp` | Feedforward somado após limite do PID → windup | PID agora reserva espaço: `SetOutputLimits(0, 100 - ff)` |
| `controle_temp.cpp` | Acumulador sigma-delta sem clamp inferior | Adicionado `if (acum < 0) acum = 0` |
| `controle_temp.cpp` | Sem timing determinístico | Loop de controle rodando a 10 Hz via `millis()` |
| `hal_io.cpp` | PWM do FAN em 8 bits (perda de resolução) | PWM configurado em 12 bits (0–4095) |
| `app.cpp` | Desligar sistema não zerava FAN | `hal_fan_write(0)` e `hal_triac_write(false)` na transição |
| `app.cpp` | `app.h` não existia | Arquivo criado |

### Melhorias

- **Detecção de falha do termopar**: leitura ADC fora da faixa desliga o aquecimento e liga o FAN no máximo
- **Anti-windup do PID**: `iAwClamp` ativado no QuickPID
- **Feedforward corrigido**: fan normalizado (0.0–1.0), teto `FF_MAX` configurável
- **Constantes documentadas** em `config.h` com cálculo do `TERMOPAR_COEF`
- **Transições de estado** explícitas em `app.cpp` (ligado→desligado e vice-versa)

---

## Hardware

### Termopar
- Amplificador: **MCP6022** (op-amp rail-to-rail)
- Ganho: `1 + R9/R8 = 1 + 150k/1k = 151`
- Pull-up de proteção: R10 (10MΩ) — puxa entrada para 3.3V se termopar abrir
- Tipo de termopar: **K** (~41 µV/°C)
- Faixa útil: 0–530°C (saturação do amp com ganho 151 e Vcc 3.3V)
- `TERMOPAR_COEF` deve ser calibrado com ponto de referência conhecido

### NTC (compensação de junta fria)
- Divisor: +3.3V → R11 (10k) → nó T-AMB → RT1 NTC (10k) → GND
- NTC no **lado baixo** do divisor

### Aquecimento
- TRIAC: **BTA24-600**
- Driver: **MOC3443** (zero-cross interno — sem necessidade de ISR)
- Controle: Sigma-Delta (ON/OFF sincronizado com zero-cross)

### FAN
- PWM: **25 kHz** (inaudível)
- Controle proporcional à temperatura

---

## Pinagem (STM32F103 / CH32V203)

| Pino | Função |
|------|--------|
| PA0 | TRIAC (saída digital) |
| PA1 | FAN PWM (25 kHz) |
| PA2 | Termopar ADC (saída MCP6022) |
| PA3 | NTC ADC (divisor R11/RT1) |
| PA4 | Chave magnética (pull-up interno) |

---

## Estrutura do projeto

```
Estacao_Solda_v2/
├── include/
│   └── config.h          ← constantes e parâmetros
├── lib/
│   ├── hal/
│   │   ├── hal_io.h
│   │   └── hal_io.cpp    ← GPIO, ADC, PWM
│   ├── control/
│   │   ├── controle_temp.h
│   │   └── controle_temp.cpp  ← PID, Feedforward, Sigma-Delta
│   └── app/
│       ├── app.h
│       └── app.cpp       ← lógica de alto nível
└── src/
    └── main.cpp
```

---

## Calibração do termopar

1. Meça a tensão de saída do MCP6022 a uma temperatura conhecida (ex: 100°C com termômetro de referência)
2. Calcule: `TERMOPAR_COEF = T_referencia / V_medida`
3. Atualize o valor em `config.h`

---

## Melhorias futuras

- [ ] Autotune do PID
- [ ] Interface OLED + encoder rotativo
- [ ] Perfis de temperatura (lead-free, etc.)
- [ ] Cooldown automático com curva
- [ ] LUT para linearização do termopar tipo K
- [ ] Log via Serial para tuning

---

## Plataforma

| Target | Board | Framework |
|--------|-------|-----------|
| STM32F103C8 | bluepill_f103c8 | Arduino (STM32duino) |
| CH32V203C8T6 | ch32v203c8t6 | Arduino (ch32v) |

Upload STM32: ST-Link  
Monitor serial: 115200 baud
