#include "ActuadorBase.h"

ActuadorBase::ActuadorBase(String id, String lugar) {

    id_dispositivo = id;
    ubicacion = lugar;

    estado_activo = false;
    potencia_pwm = 0;

}

bool ActuadorBase::conectar() {

    return true;

}

void ActuadorBase::enviar_telemetria() {

    Serial.println("Actuador: " + id_dispositivo);

    Serial.println("Ubicacion: " + ubicacion);

    Serial.println("Estado: " +
        String(estado_activo ? "Activo" : "Inactivo"));

}

String ActuadorBase::getId() {
    return id_dispositivo;
}

String ActuadorBase::getUbicacion() {
    return ubicacion;
}

bool ActuadorBase::getEstado() {
    return estado_activo;
}

int ActuadorBase::getPotencia() {
    return potencia_pwm;
}

void ActuadorBase::setUbicacion(String lugar) {

    ubicacion = lugar;

}

bool ActuadorBase::setPotencia(int valor) {

    if (valor < 0 || valor > 255) {
        return false;
    }

    potencia_pwm = valor;

    return true;
}

ActuadorBase::~ActuadorBase() {}
