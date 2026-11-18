
#include "SensorBase.h"
#include <math.h>

SensorBase::SensorBase(String id, String lugar)
    : NodoIoT(id, "Sin IP", lugar),
      limite_alerta(0) {
}

void SensorBase::enviar_telemetria() {

    Serial.println("Sensor: " + id_dispositivo);
    Serial.println("Ubicacion: " + ubicacion);
    Serial.println("Lectura: " + leer_sensor());

}

float SensorBase::getLimiteAlerta() const {

    return limite_alerta;

}

bool SensorBase::setLimiteAlerta(float valor) {

    if (!isfinite(valor) || valor < 0 || valor > 100) {
        return false;
    }

    limite_alerta = valor;
    return true;
}
