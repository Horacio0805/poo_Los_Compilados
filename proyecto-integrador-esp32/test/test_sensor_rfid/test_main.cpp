
#include <Arduino.h>
#include <SPI.h>
#include <unity.h>
#include "SensorRFID.h"

// ============================================
// CONFIGURACION RFID
// ============================================

#define RFID_SS  5
#define RFID_RST 22

const String UID_AUTORIZADO = "25 2F E3 BE";

SensorRFID sensorRFID(UID_AUTORIZADO);

// Funciones requeridas por Unity
void setUp(void) {}
void tearDown(void) {}

// ============================================
// PRUEBA 1: TARJETA AUTORIZADA
// ============================================

void test_tarjeta_autorizada() {

    TEST_ASSERT_TRUE(
        sensorRFID.validar_uid("25 2F E3 BE")
    );
}

// ============================================
// PRUEBA 2: TARJETA NO AUTORIZADA
// ============================================

void test_tarjeta_no_autorizada() {

    TEST_ASSERT_FALSE(
        sensorRFID.validar_uid("A1 B2 C3 D4")
    );
}

// ============================================
// PRUEBA 3: UID VACIO
// ============================================

void test_uid_vacio() {

    TEST_ASSERT_FALSE(
        sensorRFID.validar_uid("")
    );
}

// ============================================
// PRUEBA 4: LECTURA FISICA DEL RFID
// ============================================

void test_lectura_fisica_rfid() {

    Serial.println("Acerca tu tarjeta autorizada");
    Serial.println("Tiempo disponible: 15 segundos");

    unsigned long inicio = millis();
    String uidLeido = "";

    while (millis() - inicio < 15000) {

        uidLeido = sensorRFID.leer_tarjeta();

        if (uidLeido.length() > 0) {
            break;
        }

        yield();
    }

    Serial.print("UID detectado: ");
    Serial.println(uidLeido);

    TEST_ASSERT_TRUE(uidLeido.length() > 0);

    TEST_ASSERT_TRUE(
        sensorRFID.validar_uid(uidLeido)
    );
}

// ============================================
// SETUP DE PRUEBAS
// ============================================

void setup() {

    Serial.begin(115200);

    SPI.begin(18, 19, 23, RFID_SS);

    sensorRFID.conectar();

    UNITY_BEGIN();

    RUN_TEST(test_tarjeta_autorizada);
    RUN_TEST(test_tarjeta_no_autorizada);
    RUN_TEST(test_uid_vacio);
    RUN_TEST(test_lectura_fisica_rfid);

    UNITY_END();
}

void loop() {}
