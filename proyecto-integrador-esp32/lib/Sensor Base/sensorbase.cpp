#include "SensorBase.h"

SensorBase::SensorBase(String id, String lugar) {

    id_dispositivo = id;
    ubicacion = lugar;
    limite_alerta = 0;
    conectado = false;

}

bool SensorBase::conectar() {

    conectado = true;

    return conectado;
}

void SensorBase::enviar_telemetria() {

    Serial.println("ID: " + id_dispositivo);

    Serial.println("Ubicacion: " + ubicacion);

    Serial.println("Conexion: " +
        String(conectado ? "Activa" : "Inactiva"));

}

String SensorBase::getId() {
    return id_dispositivo;
}

String SensorBase::getUbicacion() {
    return ubicacion;
}

float SensorBase::getLimiteAlerta() {
    return limite_alerta;
}

bool SensorBase::getEstadoConexion() {
    return conectado;
}

void SensorBase::setUbicacion(String lugar) {

    ubicacion = lugar;

}

bool SensorBase::setLimiteAlerta(float valor) {

    if (!isfinite(valor) || valor < 0 || valor > 100) {
        return false;
    }

    limite_alerta = valor;

    return true;
}

SensorBase::~SensorBase() {}
