
#ifndef ACTUADOR_CERRADURA_H
#define ACTUADOR_CERRADURA_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "../ActuadorBase/ActuadorBase.h"

class ActuadorCerradura : public ActuadorBase {

private:

    Servo servo;
    int pin_servo;
    int angulo_abierto;
    int angulo_cerrado;

public:

    ActuadorCerradura(int pin);

    bool conectar() override;

    void abrir_cerradura();

    void cerrar_cerradura();

    void activar() override;

    void desactivar() override;

    int getAngulo();

};

#endif
