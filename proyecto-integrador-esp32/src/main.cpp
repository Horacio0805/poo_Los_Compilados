
#include <Arduino.h>
#include <SPI.h>

#include "../lib/SensorRFID/SensorRFID.h"
#include "../lib/SensorPuerta/SensorPuerta.h"
#include "../lib/ActuadorCerradura/ActuadorCerradura.h"
#include "../lib/ActuadorAlarma/ActuadorAlarma.h"

// CONFIGURACION DE PINES

#define RFID_SS      5
#define RFID_RST     22
#define PUERTA_PIN   27
#define SERVO_PIN    25
#define BUZZER_PIN   26
#define LED_VERDE    32
#define LED_ROJO     33

// Cambiar por el UID real de tu tarjeta
const String UID_AUTORIZADO = "A1 B2 C3 D4";

// CREACION DE OBJETOS

SensorRFID rfid(UID_AUTORIZADO);
SensorPuerta sensorPuerta(PUERTA_PIN);
ActuadorCerradura cerradura(SERVO_PIN);
ActuadorAlarma alarma(BUZZER_PIN, 500);

// VARIABLES DE CONTROL

bool accesoActivo = false;
bool puertaFueAbierta = false;
bool alertaPuertaEmitida = false;

unsigned long inicioAcceso = 0;
unsigned long inicioPuertaAbierta = 0;

const unsigned long TIEMPO_ACCESO = 5000;
const unsigned long TIEMPO_MAX_ABIERTO = 15000;

// FUNCIONES AUXILIARES

void bloquearAcceso() {

    cerradura.cerrar_cerradura();

    digitalWrite(LED_VERDE, LOW);

    accesoActivo = false;
    puertaFueAbierta = false;

    Serial.println("ACCESO FINALIZADO");

}

void autorizarAcceso() {

    if (accesoActivo) {
        Serial.println("Acceso ya activo");
        return;
    }

    cerradura.abrir_cerradura();

    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ROJO, LOW);

    accesoActivo = true;
    puertaFueAbierta = false;
    alertaPuertaEmitida = false;

    inicioAcceso = millis();

    Serial.println("ACCESO AUTORIZADO");

}

void denegarAcceso() {

    Serial.println("ACCESO DENEGADO");

    digitalWrite(LED_ROJO, HIGH);

    alarma.emitir_alerta();

}

void revisarTarjeta() {

    String uid = rfid.leer_tarjeta();

    if (uid == "") {
        return;
    }

    Serial.println("Tarjeta detectada: " + uid);

    if (rfid.validar_uid(uid)) {

        autorizarAcceso();

    } else {

        denegarAcceso();

    }

}

void supervisarPuerta() {

    bool puertaAbierta = sensorPuerta.esta_abierta();

    if (!accesoActivo) {
        return;
    }

    unsigned long ahora = millis();

    if (puertaAbierta && !puertaFueAbierta) {

        puertaFueAbierta = true;
        inicioPuertaAbierta = ahora;

        Serial.println("Puerta abierta");

    }

    // Bloquear al cerrar despues de abrir
    if (puertaFueAbierta && !puertaAbierta) {

        bloquearAcceso();
        return;

    }

    // Tiempo agotado sin abrir
    if (!puertaFueAbierta &&
        ahora - inicioAcceso >= TIEMPO_ACCESO) {

        Serial.println("Tiempo de acceso agotado");

        bloquearAcceso();
        return;

    }

    // Puerta abierta demasiado tiempo
    if (puertaFueAbierta &&
        puertaAbierta &&
        !alertaPuertaEmitida &&
        ahora - inicioPuertaAbierta >= TIEMPO_MAX_ABIERTO) {

        Serial.println("ALERTA: Puerta abierta demasiado tiempo");

        alarma.emitir_alerta();

        alertaPuertaEmitida = true;

    }

}

void actualizarIndicadores() {

    // Mantener el LED rojo encendido
    // mientras la alarma esta activa
    digitalWrite(
        LED_ROJO,
        alarma.getEstadoAlarma() ? HIGH : LOW
    );

}

// SETUP

void setup() {

    Serial.begin(115200);

    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, LOW);

    SPI.begin(18, 19, 23, RFID_SS);

    rfid.conectar();
    sensorPuerta.conectar();
    cerradura.conectar();
    alarma.conectar();

    Serial.println("==============================");
    Serial.println("CONTROL DE ACCESO CON RFID");
    Serial.println("ETAPA 2 - POO");
    Serial.println("ESP32 - Sistema iniciado");
    Serial.println("==============================");

    Serial.println("Dispositivos inicializados");

}

// LOOP PRINCIPAL

void loop() {

    revisarTarjeta();

    supervisarPuerta();

    alarma.actualizar();

    actualizarIndicadores();

}
