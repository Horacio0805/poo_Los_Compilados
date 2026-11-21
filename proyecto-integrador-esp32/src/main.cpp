
/*
 * ==========================================================
 * PROYECTO: SISTEMA DE CONTROL DE ACCESO RFID CON ESP32
 * MATERIA: PROGRAMACION ORIENTADA A OBJETOS
 * ETAPA: 2
 * ==========================================================
 *
 * DESCRIPCION:
 *
 * Sistema de control de acceso que utiliza un lector RFID
 * para autorizar la apertura de una cerradura controlada
 * mediante un servomotor.
 *
 * El sistema incorpora un sensor magnetico para supervisar
 * el estado de la puerta, un buzzer para emitir alertas
 * y dos indicadores LED para mostrar el estado del acceso.
 *
 * Se utilizan clases personalizadas para separar las
 * funciones de los sensores y actuadores.
 *
 * Los tiempos de funcionamiento se administran mediante
 * millis(), evitando bloqueos por delay().
 * ==========================================================
 */

#include <Arduino.h>
#include <SPI.h>

// Librerias personalizadas del proyecto
#include "../lib/SensorRFID/SensorRFID.h"
#include "../lib/SensorPuerta/SensorPuerta.h"
#include "../lib/ActuadorCerradura/ActuadorCerradura.h"
#include "../lib/ActuadorAlarma/ActuadorAlarma.h"

// ==========================================================
// 1. CONFIGURACION DE PINES
// ==========================================================

// Pines de comunicacion con el modulo RFID RC522
#define RFID_SS      5
#define RFID_RST     22

// Entrada del sensor magnetico
#define PUERTA_PIN   27

// Salida para el servomotor Steren MOT-100
#define SERVO_PIN    14

// Salida para el buzzer
#define BUZZER_PIN   26

// Salidas de los indicadores visuales
#define LED_VERDE    32
#define LED_ROJO     33

// UID registrado como tarjeta autorizada
const String UID_AUTORIZADO = "25 2F E3 BE";

// ==========================================================
// 2. CREACION DE OBJETOS
// ==========================================================

/*
 * Cada dispositivo se representa mediante un objeto
 * de su clase correspondiente.
 *
 * Esto permite mantener separadas las operaciones
 * de lectura, validacion y accionamiento.
 */

SensorRFID rfid(UID_AUTORIZADO);

SensorPuerta sensorPuerta(PUERTA_PIN);

ActuadorCerradura cerradura(SERVO_PIN);

ActuadorAlarma alarma(BUZZER_PIN, 500);

// ==========================================================
// 3. CONFIGURACION DE TIEMPOS
// ==========================================================

// Tiempo disponible para abrir despues de autorizar RFID
const unsigned long TIEMPO_ACCESO = 5000;

// Tiempo maximo recomendado con la puerta abierta
const unsigned long TIEMPO_MAX_ABIERTO = 15000;

// Duracion de la alarma por apertura no autorizada
const unsigned long DURACION_ALARMA_INTRUSION = 10000;

// Espera antes de bloquear nuevamente la chapa
const unsigned long RETARDO_CIERRE = 1000;

// Duracion de cada pitido de autorizacion
const unsigned long DURACION_PITIDO = 120;

// Pausa entre los dos pitidos
const unsigned long PAUSA_PITIDO = 120;

// Intervalo de parpadeo del LED rojo
const unsigned long INTERVALO_PARPADEO = 250;

// ==========================================================
// 4. VARIABLES DE CONTROL
// ==========================================================

/*
 * Estas variables permiten conocer el estado del sistema
 * y tomar decisiones de acuerdo con los eventos detectados.
 */

// Indica si existe una autorizacion RFID vigente
bool accesoActivo = false;

// Registra si la puerta fue abierta despues de autorizar
bool puertaFueAbierta = false;

// Evita repetir la alerta por puerta abierta mucho tiempo
bool alertaPuertaEmitida = false;

// Almacena el estado anterior del sensor magnetico
bool puertaAbiertaAnterior = false;

// Control del cierre automatico
bool cierrePendiente = false;
unsigned long inicioEsperaCierre = 0;

// Control de la alarma por intrusion
bool alarmaIntrusionActiva = false;
unsigned long inicioAlarmaIntrusion = 0;

// Control de la secuencia de dos pitidos
bool pitidosActivos = false;
uint8_t fasePitido = 0;
unsigned long tiempoPitido = 0;

// Temporizadores de acceso y apertura
unsigned long inicioAcceso = 0;
unsigned long inicioPuertaAbierta = 0;

// ==========================================================
// 5. CONFIRMACION AUDITIVA DE ACCESO
// ==========================================================

/*
 * Inicia una secuencia de dos pitidos cortos cuando
 * se reconoce una tarjeta autorizada.
 *
 * Si existe una alarma de intrusion activa,
 * no se interrumpe para reproducir los pitidos.
 */

void iniciarPitidosAutorizacion() {

    if (alarmaIntrusionActiva) {
        return;
    }

    pitidosActivos = true;
    fasePitido = 1;
    tiempoPitido = millis();

    // Iniciar primer pitido
    alarma.activar();

    Serial.println("Dos pitidos de autorizacion");
}

/*
 * Controla los dos pitidos mediante una maquina
 * de estados sencilla.
 *
 * Fase 1: primer pitido.
 * Fase 2: pausa.
 * Fase 3: segundo pitido.
 *
 * Se utiliza millis() para no detener el programa.
 */

void actualizarPitidos() {

    if (!pitidosActivos || alarmaIntrusionActiva) {
        return;
    }

    unsigned long ahora = millis();

    switch (fasePitido) {

        case 1:

            // Finalizar primer pitido despues de 120 ms
            if (ahora - tiempoPitido >= DURACION_PITIDO) {

                alarma.desactivar();

                fasePitido = 2;
                tiempoPitido = ahora;
            }
            break;

        case 2:

            // Esperar 120 ms antes del segundo pitido
            if (ahora - tiempoPitido >= PAUSA_PITIDO) {

                alarma.activar();

                fasePitido = 3;
                tiempoPitido = ahora;
            }
            break;

        case 3:

            // Finalizar segundo pitido
            if (ahora - tiempoPitido >= DURACION_PITIDO) {

                alarma.desactivar();

                fasePitido = 0;
                pitidosActivos = false;
            }
            break;
    }
}

// ==========================================================
// 6. BLOQUEO DE LA CERRADURA
// ==========================================================

/*
 * Finaliza el acceso autorizado.
 *
 * El servomotor regresa a su posicion inicial de 0 grados
 * y las variables del sistema se restablecen.
 */

void bloquearAcceso() {

    // Regresar el servomotor a la posicion de bloqueo
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

// ==========================================================
// 7. AUTORIZACION DE ACCESO
// ==========================================================

/*
 * Se ejecuta cuando el lector RFID reconoce una tarjeta
 * cuyo UID coincide con el registrado.
 *
 * Desbloquea la cerradura, activa el estado autorizado
 * y genera dos pitidos como confirmacion.
 */

void autorizarAcceso() {

    // No permitir la apertura durante una alarma activa
    if (alarmaIntrusionActiva) {

        Serial.println(
            "Alarma activa: acceso temporalmente bloqueado"
        );

        return;
    }

    // Evitar repetir la apertura si ya existe autorizacion
    if (accesoActivo) {

        Serial.println("Acceso ya activo");
        return;
    }

    // Mover el servomotor a 90 grados
    cerradura.abrir_cerradura();

    // Registrar nuevo estado del acceso
    accesoActivo = true;
    puertaFueAbierta = false;
    alertaPuertaEmitida = false;
    cierrePendiente = false;

    // Iniciar temporizador de autorizacion
    inicioAcceso = millis();

    // Emitir dos pitidos cortos
    iniciarPitidosAutorizacion();

    Serial.println("==========================");
    Serial.println("ACCESO AUTORIZADO");
    Serial.println("Servo enviado a 90 grados");
    Serial.println("LED VERDE ENCENDIDO");
    Serial.println("==========================");
}

// ==========================================================
// 8. ACCESO DENEGADO
// ==========================================================

/*
 * Se ejecuta cuando la tarjeta detectada no coincide
 * con el UID autorizado.
 *
 * El sistema mantiene bloqueada la cerradura
 * y emite una alerta breve.
 */

void denegarAcceso() {

    Serial.println("ACCESO DENEGADO");
    Serial.println("Tarjeta no autorizada");

    // No interrumpir otras alertas en ejecucion
    if (!alarmaIntrusionActiva && !pitidosActivos) {

        alarma.emitir_alerta();
    }
}

// ==========================================================
// 9. LECTURA Y VALIDACION DE TARJETAS RFID
// ==========================================================

/*
 * Solicita al objeto SensorRFID la lectura de una tarjeta.
 *
 * Si detecta un UID, verifica si corresponde al autorizado
 * y ejecuta la funcion de acceso correspondiente.
 */

void revisarTarjeta() {

    String uid = rfid.leer_tarjeta();

    // No realizar acciones si no existe una tarjeta detectada
    if (uid == "") {
        return;
    }

    Serial.println("Tarjeta detectada: " + uid);

    // Validar el UID mediante el objeto SensorRFID
    if (rfid.validar_uid(uid)) {

        autorizarAcceso();

    } else {

        denegarAcceso();
    }
}

// ==========================================================
// 10. DETECCION DE APERTURA NO AUTORIZADA
// ==========================================================

/*
 * Supervisa el sensor magnetico para identificar
 * aperturas de la puerta sin autorizacion RFID.
 *
 * Si se detecta una nueva apertura no autorizada,
 * activa el buzzer durante 10 segundos.
 *
 * La alarma tiene prioridad sobre los pitidos normales.
 */

void detectarIntrusion(bool puertaAbierta) {

    unsigned long ahora = millis();

    // Detectar cambio de puerta cerrada a abierta
    // cuando no existe una autorizacion vigente
    if (puertaAbierta &&
        !puertaAbiertaAnterior &&
        !accesoActivo &&
        !alarmaIntrusionActiva) {

        // Cancelar la secuencia de pitidos
        pitidosActivos = false;
        fasePitido = 0;

        // Registrar inicio de la intrusion
        alarmaIntrusionActiva = true;
        inicioAlarmaIntrusion = ahora;

        // Activar buzzer
        alarma.activar();

        Serial.println("==========================");
        Serial.println("ALERTA DE SEGURIDAD");
        Serial.println("PUERTA ABIERTA SIN RFID");
        Serial.println("BUZZER ACTIVADO: 10 SEG");
        Serial.println("LED ROJO PARPADEANDO");
        Serial.println("==========================");
    }

    // Finalizar la alarma al transcurrir 10 segundos
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

    // Conservar el estado para detectar nuevas aperturas
    puertaAbiertaAnterior = puertaAbierta;
}

// ==========================================================
// 11. SUPERVISION DEL SENSOR MAGNETICO
// ==========================================================

/*
 * Controla el comportamiento de la cerradura de acuerdo
 * con el estado del sensor magnetico.
 *
 * Despues de validar una tarjeta, el servo permanece
 * desbloqueado mientras la puerta este abierta.
 *
 * Al detectar que la puerta se cerro, espera un segundo
 * antes de regresar el servo a su posicion inicial.
 *
 * Si la puerta se abre durante la espera,
 * el cierre pendiente se cancela.
 */

void supervisarPuerta(bool puertaAbierta) {

    unsigned long ahora = millis();

    // Supervisar continuamente posibles intrusiones
    detectarIntrusion(puertaAbierta);

    // Si no existe autorizacion, finalizar supervision normal
    if (!accesoActivo) {
        return;
    }

    // Detectar la primera apertura despues de autorizar
    if (puertaAbierta && !puertaFueAbierta) {

        puertaFueAbierta = true;
        inicioPuertaAbierta = ahora;

        Serial.println("==========================");
        Serial.println("PUERTA ABIERTA");
        Serial.println("Acceso autorizado");
        Serial.println("Servo permanece a 90 grados");
        Serial.println("==========================");
    }

    // Detectar cuando el sensor magnetico indica cierre
    if (puertaFueAbierta &&
        !puertaAbierta &&
        !cierrePendiente) {

        cierrePendiente = true;

        // Iniciar espera de un segundo
        inicioEsperaCierre = ahora;

        Serial.println("==========================");
        Serial.println("PUERTA CERRADA");
        Serial.println("Esperando 1 segundo");
        Serial.println("==========================");
    }

    // Cancelar el bloqueo si la puerta vuelve a abrirse
    if (cierrePendiente && puertaAbierta) {

        cierrePendiente = false;

        Serial.println(
            "Puerta reabierta: cierre cancelado"
        );
    }

    // Bloquear un segundo despues del cierre magnetico
    if (cierrePendiente &&
        !puertaAbierta &&
        ahora - inicioEsperaCierre >= RETARDO_CIERRE) {

        bloquearAcceso();
        return;
    }

    // Cancelar autorizacion si no se abre en 5 segundos
    if (!puertaFueAbierta &&
        ahora - inicioAcceso >= TIEMPO_ACCESO) {

        Serial.println("Tiempo de acceso agotado");

        bloquearAcceso();
        return;
    }

    // Emitir advertencia por puerta abierta demasiado tiempo
    if (puertaFueAbierta &&
        puertaAbierta &&
        !alertaPuertaEmitida &&
        ahora - inicioPuertaAbierta >=
        TIEMPO_MAX_ABIERTO) {

        Serial.println(
            "ALERTA: PUERTA ABIERTA MAS DE 15 SEG"
        );

        // Evitar interferir con otros avisos del buzzer
        if (!alarmaIntrusionActiva && !pitidosActivos) {

            alarma.emitir_alerta();
        }

        // Evitar que la advertencia se repita continuamente
        alertaPuertaEmitida = true;
    }
}

// ==========================================================
// 12. ACTUALIZACION DEL BUZZER
// ==========================================================

/*
 * Actualiza el temporizador interno del buzzer.
 *
 * Durante una intrusion o los dos pitidos de autorizacion,
 * se evita que el temporizador normal interrumpa la señal.
 */

void actualizarAlarmas() {

    if (!alarmaIntrusionActiva && !pitidosActivos) {

        alarma.actualizar();
    }
}

// ==========================================================
// 13. INDICADORES LED
// ==========================================================

/*
 * Los LEDs muestran el estado logico del acceso.
 *
 * ROJO FIJO: cerradura bloqueada.
 * VERDE FIJO: cerradura desbloqueada.
 * ROJO PARPADEANDO: intrusion detectada.
 *
 * La intrusion tiene prioridad visual.
 */

void actualizarIndicadores() {

    unsigned long ahora = millis();

    // Estado de intrusion
    if (alarmaIntrusionActiva) {

        // Alternar LED rojo cada 250 milisegundos
        bool estadoRojo =
            ((ahora / INTERVALO_PARPADEO) % 2) == 0;

        digitalWrite(
            LED_ROJO,
            estadoRojo ? HIGH : LOW
        );

        digitalWrite(LED_VERDE, LOW);
    }

    // Estado de acceso autorizado
    else if (accesoActivo) {

        digitalWrite(LED_ROJO, LOW);
        digitalWrite(LED_VERDE, HIGH);
    }

    // Estado de cerradura bloqueada
    else {

        digitalWrite(LED_ROJO, HIGH);
        digitalWrite(LED_VERDE, LOW);
    }
}

// ==========================================================
// 14. INICIALIZACION DEL SISTEMA
// ==========================================================

/*
 * Configura la comunicacion serial, los indicadores LED
 * y los dispositivos conectados a la ESP32.
 *
 * La inicializacion de cada componente se realiza mediante
 * el metodo conectar() de su clase correspondiente.
 */

void setup() {

    Serial.begin(115200);

    // Configurar los LEDs como salidas digitales
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    // Estado visual inicial: cerradura bloqueada
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);

    // Configurar comunicacion SPI con el lector RFID
    SPI.begin(18, 19, 23, RFID_SS);

    // Inicializar los sensores
    rfid.conectar();
    sensorPuerta.conectar();

    // Inicializar el servomotor
    bool servoConectado = cerradura.conectar();

    // Inicializar buzzer
    alarma.conectar();

    // Registrar estado inicial del sensor magnetico
    puertaAbiertaAnterior = sensorPuerta.esta_abierta();

    // Mostrar informacion del sistema
    Serial.println("==========================");
    Serial.println("CONTROL DE ACCESO RFID");
    Serial.println("ETAPA 2 - POO");
    Serial.println("ESP32");
    Serial.println("==========================");

    // Comprobar inicializacion del control del servo
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

// ==========================================================
// 15. CICLO PRINCIPAL
// ==========================================================

/*
 * Ejecuta continuamente las funciones de supervision.
 *
 * Cada componente tiene una responsabilidad especifica:
 *
 * 1. Supervisar el sensor magnetico.
 * 2. Leer y validar tarjetas RFID.
 * 3. Actualizar los pitidos de autorizacion.
 * 4. Gestionar los temporizadores del buzzer.
 * 5. Actualizar los indicadores visuales.
 *
 * No se utiliza delay(), permitiendo que el programa
 * continue atendiendo los diferentes eventos del sistema.
 */

void loop() {

    // Leer el estado actual de la puerta
    bool puertaAbierta = sensorPuerta.esta_abierta();

    // Supervisar puerta, intrusiones y cierre de chapa
    supervisarPuerta(puertaAbierta);

    // Comprobar si se presenta una tarjeta RFID
    revisarTarjeta();

    // Actualizar la secuencia de dos pitidos
    actualizarPitidos();

    // Actualizar el funcionamiento del buzzer
    actualizarAlarmas();

    // Mostrar el estado del sistema mediante los LEDs
    actualizarIndicadores();
}
