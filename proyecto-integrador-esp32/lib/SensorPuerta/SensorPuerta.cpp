
#include "SensorPuerta.h"

SensorPuerta::SensorPuerta(int pin)
    : SensorBase("PUERTA01", "Marco de puerta"),
      puerta_abierta(false),
      pin_sensor(pin) {
}

bool SensorPuerta::conectar() {

    pinMode(pin_sensor, INPUT_PULLUP);

    return SensorBase::conectar();

}

bool SensorPuerta::esta_abierta() {

    // Contacto NC:
    // LOW = puerta cerrada
    // HIGH = puerta abierta

    puerta_abierta = (digitalRead(pin_sensor) == HIGH);

    return puerta_abierta;

}

String SensorPuerta::leer_sensor() {

    return esta_abierta() ? "Abierta" : "Cerrada";

}

bool SensorPuerta::getEstadoPuerta() {

    return puerta_abierta;

}
