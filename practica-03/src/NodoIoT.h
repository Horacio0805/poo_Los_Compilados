#ifndef NODO_IOT_H
#define NODO_IOT_H

#include <Arduino.h>

class NodoIoT {
protected:
    String idDispositivo;
    int pin;
    bool conectado;

public:
    NodoIoT(String id, int pinHardware);

    virtual void iniciar();
    virtual void ejecutarAccion() = 0;
    virtual ~NodoIoT() = default;
};

#endif
