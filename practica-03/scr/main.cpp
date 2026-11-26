#include <Arduino.h>
#include "NodoIoT.h"
#include "NodoSensorFisico.h"
#include "NodoActuadorFisico.h"

#define PIN_SENSOR 34
#define PIN_LED 23

const int N = 2;

NodoSensorFisico sensor("SENS_POT", PIN_SENSOR);

NodoActuadorFisico actuador(
    "ACT_LED", PIN_LED, &sensor
);

// Arreglo polimorfico
NodoIoT* red[N] = {
    &sensor,
    &actuador
};

unsigned long tiempoAnterior = 0;
const unsigned long intervalo = 500;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);

    sensor.setUmbralAlerta(2048);

    Serial.println("========================");
    Serial.println("PRACTICA 3 - LOS COMPILADOS");
    Serial.println("========================");

    for (int i = 0; i < N; i++) {
        red[i]->iniciar();
    }
}

void loop() {
    unsigned long tiempoActual = millis();

    if (tiempoActual - tiempoAnterior >= intervalo) {
        tiempoAnterior = tiempoActual;

        for (int i = 0; i < N; i++) {
            red[i]->ejecutarAccion();
        }

        Serial.println("------------------------");
    }
}
