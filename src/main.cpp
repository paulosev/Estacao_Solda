#include <Arduino.h>
#include "hal_io.h"
#include "app.h"

void setup()
{
    Serial.begin(115200);
    hal_io_init();
    app_init();
    Serial.println("Estacao de Ar Quente SMD v2 - OK");
}

void loop()
{
    app_update();
}
