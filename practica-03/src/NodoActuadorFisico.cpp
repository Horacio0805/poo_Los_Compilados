#include "NodoActuadorFisico.h"

NodoActuadorFisico::NodoActuadorFisico(
    String id,
    int pinHardware,
    NodoSensorFisico* sensor)
    : NodoIoT(id, pinHardware),
      _estadoActivo(false),
      _sensor(sensor) {
}

void NodoActuadorFisico::iniciar() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    conectado = true;

    Serial.println(idDispositivo + " conectado");
}

bool NodoActuadorFisico::getEstado() const {
    return _estadoActivo;
}

void NodoActuadorFisico::ejecutarAccion() {
    _estadoActivo = _sensor->hayAlerta();

    digitalWrite(pin, _estadoActivo ? HIGH : LOW);

    Serial.print(idDispositivo);
    Serial.print(" | Estado: ");
    Serial.println(_estadoActivo ? "ENCENDIDO" : "APAGADO");
}
