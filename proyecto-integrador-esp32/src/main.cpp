
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
// VARIABLES DE ESTADO
// ============================================

bool accesoActivo = false;
bool puertaFueAbierta = false;
bool alertaPuertaEmitida = false;
bool puertaAbiertaAnterior = false;

// Retardo antes de cerrar la chapa
bool cierrePendiente = false;
unsigned long inicioEsperaCierre = 0;

// Alarma de intrusion
bool alarmaIntrusionActiva = false;
unsigned long inicioAlarmaIntrusion = 0;

// Dos pitidos RFID
bool pitidosActivos = false;
uint8_t fasePitido = 0;
unsigned long tiempoPitido = 0;

// Control de tiempos
unsigned long inicioAcceso = 0;
unsigned long inicioPuertaAbierta = 0;

// ============================================
// CONSTANTES DE TIEMPO
// ============================================

const unsigned long TIEMPO_ACCESO = 5000;
const unsigned long TIEMPO_MAX_ABIERTO = 15000;
const unsigned long DURACION_ALARMA_INTRUSION = 10000;

// Esperar 1 segundo despues de cerrar puerta
const unsigned long RETARDO_CIERRE = 1000;

// Duracion de los pitidos
const unsigned long DURACION_PITIDO = 120;
const unsigned long PAUSA_PITIDO = 120;

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

    if (!pitidosActivos) {
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
// BLOQUEAR CERRADURA
// ============================================

void bloquearAcceso() {

    // Regresar servo a 0 grados
    cerradura.cerrar_cerradura();

    digitalWrite(LED_VERDE, LOW);

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

    if (accesoActivo) {
        Serial.println("Acceso ya activo");
        return;
    }

    // Abrir cerradura
    cerradura.abrir_cerradura();

    accesoActivo = true;
    puertaFueAbierta = false;
    alertaPuertaEmitida = false;
    cierrePendiente = false;

    inicioAcceso = millis();

    digitalWrite(LED_VERDE, HIGH);

    // Dos pitidos de confirmacion
    iniciarPitidosAutorizacion();

    Serial.println("==========================");
    Serial.println("ACCESO AUTORIZADO");
    Serial.println("Servo enviado a 90 grados");
    Serial.println("Chapa desbloqueada");
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
// DETECCION DE INTRUSION
// ============================================

void detectarIntrusion(bool puertaAbierta) {

    unsigned long ahora = millis();

    // Detectar apertura sin tarjeta autorizada
    if (puertaAbierta &&
        !puertaAbiertaAnterior &&
        !accesoActivo) {

        // Cancelar pitidos
        pitidosActivos = false;
        fasePitido = 0;

        // Iniciar alarma de 10 segundos
        alarmaIntrusionActiva = true;
        inicioAlarmaIntrusion = ahora;

        alarma.activar();

        Serial.println("==========================");
        Serial.println("ALERTA: PUERTA FORZADA");
        Serial.println("Apertura sin autorizacion");
        Serial.println("BUZZER ACTIVADO 10 SEG");
        Serial.println("==========================");
    }

    // Apagar alarma despues de 10 segundos
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

    // Detectar intrusiones
    detectarIntrusion(puertaAbierta);

    // Si no hay acceso autorizado,
    // no ejecutar el control normal de cerradura
    if (!accesoActivo) {
        return;
    }

    // Detectar primera apertura autorizada
    if (puertaAbierta && !puertaFueAbierta) {

        puertaFueAbierta = true;
        inicioPuertaAbierta = ahora;

        Serial.println("==========================");
        Serial.println("PUERTA ABIERTA");
        Serial.println("Acceso autorizado");
        Serial.println("Servo permanece a 90 grados");
        Serial.println("==========================");
    }

    // Detectar cierre de puerta
    if (puertaFueAbierta &&
        !puertaAbierta &&
        !cierrePendiente) {

        cierrePendiente = true;
        inicioEsperaCierre = ahora;

        Serial.println("==========================");
        Serial.println("SENSOR MAGNETICO ACTIVADO");
        Serial.println("PUERTA CERRADA");
        Serial.println("Esperando 1 segundo");
        Serial.println("==========================");
    }

    // Si la puerta se vuelve a abrir,
    // cancelar el cierre pendiente
    if (cierrePendiente && puertaAbierta) {

        cierrePendiente = false;

        Serial.println("==========================");
        Serial.println("PUERTA ABIERTA NUEVAMENTE");
        Serial.println("Cierre de chapa cancelado");
        Serial.println("==========================");
    }

    // Esperar 1 segundo con puerta cerrada
    // antes de regresar servo a 0 grados
    if (cierrePendiente &&
        !puertaAbierta &&
        ahora - inicioEsperaCierre >= RETARDO_CIERRE) {

        Serial.println("==========================");
        Serial.println("RETARDO FINALIZADO");
        Serial.println("Regresando servo a 0 grados");
        Serial.println("==========================");

        bloquearAcceso();
        return;
    }

    // Si no abre dentro de 5 segundos
    if (!puertaFueAbierta &&
        ahora - inicioAcceso >= TIEMPO_ACCESO) {

        Serial.println("Tiempo de acceso agotado");
        Serial.println("Bloqueando chapa");

        bloquearAcceso();
        return;
    }

    // Alerta de puerta abierta mas de 15 segundos
    if (puertaFueAbierta &&
        puertaAbierta &&
        !alertaPuertaEmitida &&
        ahora - inicioPuertaAbierta >=
        TIEMPO_MAX_ABIERTO) {

        Serial.println("==========================");
        Serial.println("ALERTA: PUERTA ABIERTA MAS DE 15 SEG");
        Serial.println("==========================");

        if (!alarmaIntrusionActiva && !pitidosActivos) {
            alarma.emitir_alerta();
        }

        alertaPuertaEmitida = true;
    }
}

// ============================================
// ACTUALIZAR ALARMAS
// ============================================

void actualizarAlarmas() {

    // Evitar interrumpir los dos pitidos
    // o la alarma de intrusion
    if (!alarmaIntrusionActiva && !pitidosActivos) {
        alarma.actualizar();
    }
}

// ============================================
// ACTUALIZAR LEDS
// ============================================

void actualizarIndicadores() {

    digitalWrite(
        LED_ROJO,
        alarma.getEstadoAlarma() ? HIGH : LOW
    );
}

// ============================================
// SETUP
// ============================================

void setup() {

    Serial.begin(115200);

    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, LOW);

    // Iniciar SPI
    SPI.begin(18, 19, 23, RFID_SS);

    // Inicializar dispositivos
    rfid.conectar();
    sensorPuerta.conectar();

    bool servoConectado = cerradura.conectar();

    alarma.conectar();

    Serial.println("==========================");
    Serial.println("CONTROL DE ACCESO RFID");
    Serial.println("ETAPA 2 - POO");
    Serial.println("ESP32");
    Serial.println("==========================");

    if (servoConectado) {
        Serial.println("Servo inicializado");
    } else {
        Serial.println("ERROR: Servo no inicializado");
    }

    // Estado inicial del sensor magnetico
    puertaAbiertaAnterior = sensorPuerta.esta_abierta();

    Serial.println("Servo conectado a GPIO 14");
    Serial.println("Retardo de cierre: 1 segundo");
    Serial.println("Alarma de intrusion: 10 segundos");
    Serial.println("Sistema listo");
    Serial.println("==========================");
}

// ============================================
// LOOP PRINCIPAL
// ============================================

void loop() {

    // Supervisar estado de la puerta
    bool puertaAbierta = sensorPuerta.esta_abierta();

    supervisarPuerta(puertaAbierta);

    // Revisar tarjeta RFID
    revisarTarjeta();

    // Ejecutar dos pitidos
    actualizarPitidos();

    // Actualizar alarmas
    actualizarAlarmas();

    // Actualizar LEDs
    actualizarIndicadores();
}
