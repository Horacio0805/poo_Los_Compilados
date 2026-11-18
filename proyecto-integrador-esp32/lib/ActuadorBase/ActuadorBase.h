
#ifndef ACTUADOR_BASE_H
#define ACTUADOR_BASE_H

#include <Arduino.h>
#include "../NodoIoT/NodoIoT.h"

class ActuadorBase : public NodoIoT {

protected:
    bool estado_activo;
    int potencia_pwm;

public:
    ActuadorBase(String id, String lugar);

    virtual void activar() = 0;
    virtual void desactivar() = 0;

    void enviar_telemetria() override;

    bool getEstado() const;
    int getPotencia() const;

    bool setPotencia(int valor);

    virtual ~ActuadorBase() = default;
};

#endif

