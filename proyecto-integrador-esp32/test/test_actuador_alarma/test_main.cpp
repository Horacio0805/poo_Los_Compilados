
#include <Arduino.h>
#include <unity.h>
#include "ActuadorAlarma.h"

// ============================================
// CONFIGURACION
// ============================================

#define BUZZER_PIN 26

ActuadorAlarma alarma(BUZZER_PIN, 500);

bool inicializado = false;
bool activacionCorrecta = false;
bool desactivacionCorrecta = false;

uint8_t etapa = 0;
unsigned long inicioEtapa = 0;

// ============================================
// UNITY
// ============================================

void setUp(void) {}
void tearDown(void) {}

void test_inicializacion() {
    TEST_ASSERT_TRUE(inicializado);
}

void test_activacion() {
    TEST_ASSERT_TRUE(activacionCorrecta);
}

void test_desactivacion() {
    TEST_ASSERT_TRUE(desactivacionCorrecta);
}

// ============================================
// SETUP
// ============================================

void setup() {

    Serial.begin(115200);

    UNITY_BEGIN();

    inicializado = alarma.conectar();

    RUN_TEST(test_inicializacion);

    alarma.activar();

    activacionCorrecta = alarma.getEstadoAlarma();

    RUN_TEST(test_activacion);

    inicioEtapa = millis();
    etapa = 1;
}

// ============================================
// LOOP SIN DELAY
// ============================================

void loop() {

    unsigned long ahora = millis();

    // Mantener buzzer activo 10 segundos
    if (etapa == 1 &&
        ahora - inicioEtapa >= 10000) {

        alarma.desactivar();

        desactivacionCorrecta =
            !alarma.getEstadoAlarma();

        RUN_TEST(test_desactivacion);

        UNITY_END();

        etapa = 2;
    }
}
