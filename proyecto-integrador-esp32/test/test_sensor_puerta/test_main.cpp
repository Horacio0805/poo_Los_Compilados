
#include <Arduino.h>
#include <unity.h>
#include "SensorPuerta.h"

// ============================================
// CONFIGURACION
// ============================================

#define PUERTA_PIN 27

SensorPuerta sensorPuerta(PUERTA_PIN);

// Funciones requeridas por Unity
void setUp(void) {}
void tearDown(void) {}

// ============================================
// PRUEBA 1: DETECTAR PUERTA ABIERTA Y CERRADA
// ============================================

void test_lectura_magnetica() {

    Serial.println("PRUEBA DEL SENSOR MAGNETICO");
    Serial.println("Abre y cierra la puerta");
    Serial.println("Tiempo disponible: 15 segundos");

    bool detectoAbierta = false;
    bool detectoCerrada = false;

    bool estadoAnterior = sensorPuerta.esta_abierta();

    unsigned long inicio = millis();

    while (millis() - inicio < 15000) {

        bool estadoActual = sensorPuerta.esta_abierta();

        if (estadoActual) {
            detectoAbierta = true;
        } else {
            detectoCerrada = true;
        }

        if (estadoActual != estadoAnterior) {

            Serial.println(
                estadoActual
                ? "PUERTA ABIERTA"
                : "PUERTA CERRADA"
            );

            estadoAnterior = estadoActual;
        }

        yield();
    }

    TEST_ASSERT_TRUE(detectoAbierta);
    TEST_ASSERT_TRUE(detectoCerrada);
}

// ============================================
// PRUEBA 2: VERIFICAR ESTADO INTERNO
// ============================================

void test_estado_interno() {

    bool lectura = sensorPuerta.esta_abierta();

    TEST_ASSERT_EQUAL(
        lectura,
        sensorPuerta.getEstadoPuerta()
    );
}

// ============================================
// CONFIGURACION DE PRUEBAS
// ============================================

void setup() {

    Serial.begin(115200);

    sensorPuerta.conectar();

    UNITY_BEGIN();

    RUN_TEST(test_lectura_magnetica);
    RUN_TEST(test_estado_interno);

    UNITY_END();
}

void loop() {}
