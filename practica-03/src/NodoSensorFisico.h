#ifndef NODO_SENSOR_FISICO_H
#define NODO_SENSOR_FISICO_H

#include "NodoIoT.h"

class NodoSensorFisico : public NodoIoT {
private:
    int _umbralAlerta;
    int _valorActual;

public:
    NodoSensorFisico(String id, int pinHardware);

    void setUmbralAlerta(int umbral);
    int getValorActual() const;
    bool hayAlerta() const;

    void iniciar() override;
    void ejecutarAccion() override;
};

#endif
