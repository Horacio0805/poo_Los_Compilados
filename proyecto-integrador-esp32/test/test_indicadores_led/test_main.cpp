
#include <Arduino.h>
#include <unity.h>

// ============================================
// CONFIGURACION DE PINES
// ============================================

#define LED_VERDE 32
#define LED_ROJO  33

const unsigned long TIEMPO_PRUEBA = 2000;

unsigned long inicioEtapa = 0;
uint8_t etapa = 0;

// ============================================
// FUNCIONES UNITY
// ============================================

void setUp(void) {}
void tearDown(void) {}

// ============================================
// PRUEBAS
// ============================================

void test_led_rojo() {
    TEST_ASSERT_EQUAL(HIGH, digitalRead(LED_ROJO));
    TEST_ASSERT_EQUAL(LOW, digitalRead(LED_VERDE));
}

void test_led_verde() {
    TEST_ASSERT_EQUAL(HIGH, digitalRead(LED_VERDE));
    TEST_ASSERT_EQUAL(LOW, digitalRead(LED_ROJO));
}

void test_ambos_apagados() {
    TEST_ASSERT_EQUAL(LOW, digitalRead(LED_VERDE));
    TEST_ASSERT_EQUAL(LOW, digitalRead(LED_ROJO));
}

// ============================================
// SETUP
// ============================================

void setup() {

    Serial.begin(115200);

    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    UNITY_BEGIN();

    // Estado 1: Cerradura bloqueada
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);

    Serial.println("LED ROJO ENCENDIDO");

    RUN_TEST(test_led_rojo);

    inicioEtapa = millis();
    etapa = 1;
}

// ============================================
// LOOP SIN DELAY
// ============================================

void loop() {

    unsigned long ahora = millis();

    // Estado 2: Cerradura desbloqueada
    if (etapa == 1 &&
        ahora - inicioEtapa >= TIEMPO_PRUEBA) {

        digitalWrite(LED_ROJO, LOW);
        digitalWrite(LED_VERDE, HIGH);

        Serial.println("LED VERDE ENCENDIDO");

        RUN_TEST(test_led_verde);

        inicioEtapa = ahora;
        etapa = 2;
    }

    // Estado 3: Ambos apagados
    if (etapa == 2 &&
        ahora - inicioEtapa >= TIEMPO_PRUEBA) {

        digitalWrite(LED_VERDE, LOW);
        digitalWrite(LED_ROJO, LOW);

        Serial.println("AMBOS LEDS APAGADOS");

        RUN_TEST(test_ambos_apagados);

        UNITY_END();

        etapa = 3;
    }
}
// 