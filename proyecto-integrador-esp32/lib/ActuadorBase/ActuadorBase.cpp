
#include "ActuadorBase.h"

ActuadorBase::ActuadorBase(String id, String lugar)
    : NodoIoT(id, "Sin IP", lugar),
      estado_activo(false),
      potencia_pwm(0) {
}

void ActuadorBase::enviar_telemetria() {

    Serial.println("Actuador: " + id_dispositivo);

    Serial.println(
        estado_activo ? "Estado: Activo" :
                        "Estado: Inactivo"
    );
}

bool ActuadorBase::getEstado() const {

    return estado_activo;

}

int ActuadorBase::getPotencia() const {

    return potencia_pwm;

}

bool ActuadorBase::setPotencia(int valor) {

    if (valor < 0 || valor > 255) {
        return false;
    }

    potencia_pwm = valor;
    return true;
}

