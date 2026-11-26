#ifndef NODO_ACTUADOR_FISICO_H
#define NODO_ACTUADOR_FISICO_H

#include "NodoIoT.h"
#include "NodoSensorFisico.h"

class NodoActuadorFisico : public NodoIoT {
private:
    bool _estadoActivo;
    NodoSensorFisico* _sensor;

public:
    NodoActuadorFisico(
        String id,
        int pinHardware,
        NodoSensorFisico* sensor
    );

    bool getEstado() const;

    void iniciar() override;
    void ejecutarAccion() override;
};

#endif
