
#include "ActuadorAlarma.h"

ActuadorAlarma::ActuadorAlarma(
    int pin,
    unsigned long duracion
)
    : ActuadorBase("ALARMA01", "Entrada"),
      pin_buzzer(pin),
      duracion_alarma(duracion),
      inicio_alarma(0),
      alarma_activa(false) {
}

bool ActuadorAlarma::conectar() {

    pinMode(pin_buzzer, OUTPUT);

    digitalWrite(pin_buzzer, LOW);

    return ActuadorBase::conectar();

}

void ActuadorAlarma::activar() {

    digitalWrite(pin_buzzer, HIGH);

    estado_activo = true;
    alarma_activa = true;

}

void ActuadorAlarma::desactivar() {

    digitalWrite(pin_buzzer, LOW);

    estado_activo = false;
    alarma_activa = false;

}

void ActuadorAlarma::emitir_alerta() {

    if (!alarma_activa) {

        activar();

        inicio_alarma = millis();

    }

}

void ActuadorAlarma::actualizar() {

    if (alarma_activa &&
        millis() - inicio_alarma >= duracion_alarma) {

        desactivar();

    }

}

bool ActuadorAlarma::getEstadoAlarma() {

    return alarma_activa;

}
