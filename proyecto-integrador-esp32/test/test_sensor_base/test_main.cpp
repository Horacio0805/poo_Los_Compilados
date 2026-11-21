
#include <Arduino.h>
#include <unity.h>
#include "SensorBase.h"

// Clase auxiliar para probar SensorBase
class SensorPrueba : public SensorBase {
public:
    SensorPrueba()
        : SensorBase("TEST01", "Laboratorio") {}

    String leer_sensor() override {
        return "Dato de prueba";
    }
};

SensorPrueba sensor;

void setUp(void) {}
void tearDown(void) {}

// Verificar valor inicial
void test_valor_inicial() {
    TEST_ASSERT_EQUAL_FLOAT(
        0.0f, sensor.getLimiteAlerta()
    );
}

// Verificar valor permitido
void test_valor_valido() {
    TEST_ASSERT_TRUE(sensor.setLimiteAlerta(50.0f));
    TEST_ASSERT_EQUAL_FLOAT(
        50.0f, sensor.getLimiteAlerta()
    );
}

// Verificar limite inferior
void test_limite_inferior() {
    TEST_ASSERT_TRUE(sensor.setLimiteAlerta(0.0f));
}

// Verificar limite superior
void test_limite_superior() {
    TEST_ASSERT_TRUE(sensor.setLimiteAlerta(100.0f));
}

// Rechazar valor negativo
void test_valor_negativo() {
    TEST_ASSERT_FALSE(sensor.setLimiteAlerta(-10.0f));
}

// Rechazar valor mayor a 100
void test_valor_excesivo() {
    TEST_ASSERT_FALSE(sensor.setLimiteAlerta(150.0f));
}

// Comprobar herencia y polimorfismo
void test_polimorfismo() {
    SensorBase* base = &sensor;
    String lectura = base->leer_sensor();

    TEST_ASSERT_EQUAL_STRING(
        "Dato de prueba", lectura.c_str()
    );
}

void setup() {
    Serial.begin(115200);

    UNITY_BEGIN();

    RUN_TEST(test_valor_inicial);
    RUN_TEST(test_valor_valido);
    RUN_TEST(test_limite_inferior);
    RUN_TEST(test_limite_superior);
    RUN_TEST(test_valor_negativo);
    RUN_TEST(test_valor_excesivo);
    RUN_TEST(test_polimorfismo);

    UNITY_END();
}

void loop() {}
