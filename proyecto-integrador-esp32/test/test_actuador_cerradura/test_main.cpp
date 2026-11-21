
#include <Arduino.h>
#include <unity.h>
#include "ActuadorCerradura.h"

// ============================================
// CONFIGURACION DEL SERVOMOTOR
// ============================================

#define SERVO_PIN 14

ActuadorCerradura cerradura(SERVO_PIN);

// Variables de control
bool servoInicializado = false;
bool aperturaEjecutada = false;
bool cierreEjecutado = false;

uint8_t etapa = 0;
unsigned long inicioEtapa = 0;

const unsigned long TIEMPO_ESPERA = 2000;

// ============================================
// FUNCIONES DE UNITY
// ============================================

void setUp(void) {}
void tearDown(void) {}

// ============================================
// PRUEBA 1: INICIALIZACION DEL SERVO
// ============================================

void test_inicializacion() {

    TEST_ASSERT_TRUE(servoInicializado);
}

// ============================================
// PRUEBA 2: ORDENES DE APERTURA Y CIERRE
// ============================================

void test_comandos_servo() {

    TEST_ASSERT_TRUE(aperturaEjecutada);
    TEST_ASSERT_TRUE(cierreEjecutado);
}

// ============================================
// SETUP
// ============================================

void setup() {

    Serial.begin(115200);

    UNITY_BEGIN();

    Serial.println("==========================");
    Serial.println("PRUEBA DE ACTUADOR CERRADURA");
    Serial.println("Servo Steren MOT-100");
    Serial.println("GPIO 14");
    Serial.println("==========================");

    servoInicializado = cerradura.conectar();

    RUN_TEST(test_inicializacion);

    if (!servoInicializado) {
        UNITY_END();
        etapa = 3;
        return;
    }

    // Primera orden: abrir
    cerradura.abrir_cerradura();
    aperturaEjecutada = true;

    Serial.println("Servo enviado a 90 grados");

    inicioEtapa = millis();
    etapa = 1;
}

// ============================================
// LOOP NO BLOQUEANTE
// ============================================

void loop() {

    unsigned long ahora = millis();

    // Esperar 2 segundos antes de cerrar
    if (etapa == 1 &&
        ahora - inicioEtapa >= TIEMPO_ESPERA) {

        cerradura.cerrar_cerradura();
        cierreEjecutado = true;

        Serial.println("Servo enviado a 0 grados");

        inicioEtapa = ahora;
        etapa = 2;
    }

    // Finalizar prueba despues de otros 2 s
    if (etapa == 2 &&
        ahora - inicioEtapa >= TIEMPO_ESPERA) {

        RUN_TEST(test_comandos_servo);

        UNITY_END();

        etapa = 3;
    }
}
