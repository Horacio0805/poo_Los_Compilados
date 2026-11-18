
#include "ActuadorCerradura.h"

ActuadorCerradura::ActuadorCerradura(int pin)
    : ActuadorBase("SERVO01", "Cerradura"),
      pin_servo(pin),
      angulo_abierto(90),
      angulo_cerrado(0) {
}

bool ActuadorCerradura::conectar() {

    servo.setPeriodHertz(50);

    servo.attach(pin_servo, 500, 2400);

    if (!servo.attached()) {
        return false;
    }

    cerrar_cerradura();

    return ActuadorBase::conectar();

}

void ActuadorCerradura::abrir_cerradura() {

    servo.write(angulo_abierto);

    estado_activo = true;

    Serial.println("Cerradura desbloqueada");

}

void ActuadorCerradura::cerrar_cerradura() {

    servo.write(angulo_cerrado);

    estado_activo = false;

    Serial.println("Cerradura bloqueada");

}

void ActuadorCerradura::activar() {

    abrir_cerradura();

}

void ActuadorCerradura::desactivar() {

    cerrar_cerradura();

}

int ActuadorCerradura::getAngulo() {

    return estado_activo ? angulo_abierto : angulo_cerrado;

}
