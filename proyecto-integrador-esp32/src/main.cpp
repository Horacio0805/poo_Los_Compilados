
#include <Arduino.h>
#include <SPI.h>

#include "../lib/SensorRFID/SensorRFID.h"
#include "../lib/SensorPuerta/SensorPuerta.h"
#include "../lib/ActuadorCerradura/ActuadorCerradura.h"
#include "../lib/ActuadorAlarma/ActuadorAlarma.h"

// ============================================
// CONFIGURACION DE PINES
// ============================================

#define RFID_SS      5
#define RFID_RST     22
#define PUERTA_PIN   27
#define SERVO_PIN    14
#define BUZZER_PIN   26
#define LED_VERDE    32
#define LED_ROJO     33

// UID de tarjeta autorizada
const String UID_AUTORIZADO = "25 2F E3 BE";

// ============================================
// OBJETOS DEL SISTEMA
// ============================================

SensorRFID rfid(UID_AUTORIZADO);
SensorPuerta sensorPuerta(PUERTA_PIN);
ActuadorCerradura cerradura(SERVO_PIN);
ActuadorAlarma alarma(BUZZER_PIN, 500);

// ============================================
// CONSTANTES DE TIEMPO
// ============================================

const unsigned long TIEMPO_ACCESO = 5000;
const unsigned long TIEMPO_MAX_ABIERTO = 15000;
const unsigned long DURACION_ALARMA_INTRUSION = 10000;
const unsigned long RETARDO_CIERRE = 1000;

const unsigned long DURACION_PITIDO = 120;
const unsigned long PAUSA_PITIDO = 120;

// Parpadeo del LED rojo durante intrusion
const unsigned long INTERVALO_PARPADEO = 250;

// ============================================
// VARIABLES DE ESTADO
// ============================================

// Acceso y puerta
bool accesoActivo = false;
bool puertaFueAbierta = false;
bool alertaPuertaEmitida = false;
bool puertaAbiertaAnterior = false;

// Retardo para bloquear chapa
bool cierrePendiente = false;
unsigned long inicioEsperaCierre = 0;

// Intrusion
bool alarmaIntrusionActiva = false;
unsigned long inicioAlarmaIntrusion = 0;
bool intrusionPendiente = false;

// Dos pitidos RFID
bool pitidosActivos = false;
uint8_t fasePitido = 0;
unsigned long tiempoPitido = 0;

// Tiempos del acceso
unsigned long inicioAcceso = 0;
unsigned long inicioPuertaAbierta = 0;

// ============================================
// DOS PITIDOS DE AUTORIZACION
// ============================================

void iniciarPitidosAutorizacion() {

    if (alarmaIntrusionActiva) {
        return;
    }

    pitidosActivos = true;
    fasePitido = 1;
    tiempoPitido = millis();

    alarma.activar();

    Serial.println("Dos pitidos de autorizacion");
}

void actualizarPitidos() {

    if (!pitidosActivos || alarmaIntrusionActiva) {
        return;
    }

    unsigned long ahora = millis();

    switch (fasePitido) {

        case 1:
            if (ahora - tiempoPitido >= DURACION_PITIDO) {
                alarma.desactivar();
                fasePitido = 2;
                tiempoPitido = ahora;
            }
            break;

        case 2:
            if (ahora - tiempoPitido >= PAUSA_PITIDO) {
                alarma.activar();
                fasePitido = 3;
                tiempoPitido = ahora;
            }
            break;

        case 3:
            if (ahora - tiempoPitido >= DURACION_PITIDO) {
                alarma.desactivar();
                fasePitido = 0;
                pitidosActivos = false;
            }
            break;
    }
}

// ============================================
// CERRAR CERRADURA
// ============================================

void bloquearAcceso() {

    cerradura.cerrar_cerradura();

    accesoActivo = false;
    puertaFueAbierta = false;
    alertaPuertaEmitida = false;
    cierrePendiente = false;

    Serial.println("==========================");
    Serial.println("PUERTA BLOQUEADA");
    Serial.println("Servo enviado a 0 grados");
    Serial.println("==========================");
}

// ============================================
// AUTORIZAR ACCESO
// ============================================

void autorizarAcceso() {

    // Evitar desbloqueo durante una intrusion
    if (alarmaIntrusionActiva) {
        Serial.println("Alarma activa: acceso temporalmente bloqueado");
        return;
    }

    if (accesoActivo) {
        Serial.println("Acceso ya activo");
        return;
    }

    cerradura.abrir_cerradura();

    accesoActivo = true;
    puertaFueAbierta = false;
    alertaPuertaEmitida = false;
    cierrePendiente = false;

    inicioAcceso = millis();

    iniciarPitidosAutorizacion();

    Serial.println("==========================");
    Serial.println("ACCESO AUTORIZADO");
    Serial.println("Servo enviado a 90 grados");
    Serial.println("LED VERDE ENCENDIDO");
    Serial.println("==========================");
}

// ============================================
// DENEGAR ACCESO
// ============================================

void denegarAcceso() {

    Serial.println("ACCESO DENEGADO");
    Serial.println("Tarjeta no autorizada");

    if (!alarmaIntrusionActiva && !pitidosActivos) {
        alarma.emitir_alerta();
    }
}

// ============================================
// REVISAR TARJETA RFID
// ============================================

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

// ============================================
// DETECTAR INTRUSION
// ============================================

void detectarIntrusion(bool puertaAbierta) {

    unsigned long ahora = millis();

    // Detectar apertura no autorizada
    if (puertaAbierta &&
        !puertaAbiertaAnterior &&
        !accesoActivo &&
        !alarmaIntrusionActiva) {

        // Cancelar pitidos anteriores
        pitidosActivos = false;
        fasePitido = 0;

        alarmaIntrusionActiva = true;
        inicioAlarmaIntrusion = ahora;

        alarma.activar();

        Serial.println("==========================");
        Serial.println("ALERTA DE SEGURIDAD");
        Serial.println("PUERTA ABIERTA SIN RFID");
        Serial.println("BUZZER ACTIVADO: 10 SEG");
        Serial.println("LED ROJO PARPADEANDO");
        Serial.println("==========================");
    }

    // Finalizar alarma despues de 10 segundos
    if (alarmaIntrusionActiva &&
        ahora - inicioAlarmaIntrusion >=
        DURACION_ALARMA_INTRUSION) {

        alarma.desactivar();
        alarmaIntrusionActiva = false;

        Serial.println("==========================");
        Serial.println("ALARMA FINALIZADA");
        Serial.println("Buzzer apagado");
        Serial.println("==========================");
    }

    puertaAbiertaAnterior = puertaAbierta;
}

// ============================================
// SUPERVISAR PUERTA
// ============================================

void supervisarPuerta(bool puertaAbierta) {

    unsigned long ahora = millis();

    detectarIntrusion(puertaAbierta);

    if (!accesoActivo) {
        return;
    }

    // Primera apertura autorizada
    if (puertaAbierta && !puertaFueAbierta) {

        puertaFueAbierta = true;
        inicioPuertaAbierta = ahora;

        Serial.println("==========================");
        Serial.println("PUERTA ABIERTA");
        Serial.println("Acceso autorizado");
        Serial.println("Servo permanece a 90 grados");
        Serial.println("==========================");
    }

    // Sensor detecta puerta cerrada
    if (puertaFueAbierta &&
        !puertaAbierta &&
        !cierrePendiente) {

        cierrePendiente = true;
        inicioEsperaCierre = ahora;

        Serial.println("==========================");
        Serial.println("PUERTA CERRADA");
        Serial.println("Esperando 1 segundo");
        Serial.println("==========================");
    }

    // Cancelar cierre si vuelve a abrir
    if (cierrePendiente && puertaAbierta) {

        cierrePendiente = false;

        Serial.println("Puerta reabierta: cierre cancelado");
    }

    // Regresar servo a 0 grados tras 1 segundo
    if (cierrePendiente &&
        !puertaAbierta &&
        ahora - inicioEsperaCierre >= RETARDO_CIERRE) {

        bloquearAcceso();
        return;
    }

    // Bloquear si no se abre dentro de 5 segundos
    if (!puertaFueAbierta &&
        ahora - inicioAcceso >= TIEMPO_ACCESO) {

        Serial.println("Tiempo de acceso agotado");

        bloquearAcceso();
        return;
    }

    // Puerta abierta por mas de 15 segundos
    if (puertaFueAbierta &&
        puertaAbierta &&
        !alertaPuertaEmitida &&
        ahora - inicioPuertaAbierta >=
        TIEMPO_MAX_ABIERTO) {

        Serial.println("ALERTA: PUERTA ABIERTA MAS DE 15 SEG");

        if (!alarmaIntrusionActiva && !pitidosActivos) {
            alarma.emitir_alerta();
        }

        alertaPuertaEmitida = true;
    }
}

// ============================================
// ACTUALIZAR BUZZER
// ============================================

void actualizarAlarmas() {

    // No interrumpir alarma de 10 s ni pitidos
    if (!alarmaIntrusionActiva && !pitidosActivos) {
        alarma.actualizar();
    }
}

// ============================================
// ACTUALIZAR INDICADORES LED
// ============================================

void actualizarIndicadores() {

    unsigned long ahora = millis();

    // PRIORIDAD 1: Intrusion
    if (alarmaIntrusionActiva) {

        // LED rojo parpadea cada 250 ms
        bool estadoRojo =
            ((ahora / INTERVALO_PARPADEO) % 2) == 0;

        digitalWrite(
            LED_ROJO,
            estadoRojo ? HIGH : LOW
        );

        digitalWrite(LED_VERDE, LOW);
    }

    // PRIORIDAD 2: Cerradura desbloqueada
    else if (accesoActivo) {

        digitalWrite(LED_ROJO, LOW);
        digitalWrite(LED_VERDE, HIGH);
    }

    // PRIORIDAD 3: Cerradura bloqueada
    else {

        digitalWrite(LED_ROJO, HIGH);
        digitalWrite(LED_VERDE, LOW);
    }
}

// ============================================
// SETUP
// ============================================

void setup() {

    Serial.begin(115200);

    // Inicializar LEDs
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    // Cerradura inicialmente bloqueada
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);

    // Inicializar SPI
    SPI.begin(18, 19, 23, RFID_SS);

    // Inicializar componentes
    rfid.conectar();
    sensorPuerta.conectar();

    bool servoConectado = cerradura.conectar();

    alarma.conectar();

    // Leer estado inicial del sensor
    puertaAbiertaAnterior = sensorPuerta.esta_abierta();

    Serial.println("==========================");
    Serial.println("CONTROL DE ACCESO RFID");
    Serial.println("ETAPA 2 - POO");
    Serial.println("ESP32");
    Serial.println("==========================");

    if (servoConectado) {
        Serial.println("Servo inicializado: GPIO 14");
    } else {
        Serial.println("ERROR AL INICIALIZAR SERVO");
    }

    Serial.println("LED rojo: GPIO 33");
    Serial.println("LED verde: GPIO 32");
    Serial.println("Retardo de cierre: 1 segundo");
    Serial.println("Alarma de intrusion: 10 segundos");
    Serial.println("Sistema listo");
    Serial.println("==========================");
}

// ============================================
// LOOP PRINCIPAL
// ============================================

void loop() {

    // Supervisar puerta primero
    bool puertaAbierta = sensorPuerta.esta_abierta();

    supervisarPuerta(puertaAbierta);

    // Revisar tarjetas
    revisarTarjeta();

    // Ejecutar pitidos
    actualizarPitidos();

    // Actualizar alarma
    actualizarAlarmas();

    // Actualizar LEDs
    actualizarIndicadores();
}
