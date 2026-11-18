
#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

#include <Arduino.h>
#include "../NodoIoT/NodoIoT.h"

class SensorBase : public NodoIoT {

private:
    float limite_alerta;

public:
    SensorBase(String id, String lugar);

    virtual String leer_sensor() = 0;

    void enviar_telemetria() override;

    float getLimiteAlerta() const;

    bool setLimiteAlerta(float valor);

    virtual ~SensorBase() = default;
};

#endif

