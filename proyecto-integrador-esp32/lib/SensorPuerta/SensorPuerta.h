
#ifndef SENSOR_PUERTA_H
#define SENSOR_PUERTA_H

#include <Arduino.h>
#include "../SensorBase/SensorBase.h"

class SensorPuerta : public SensorBase {

private:

    bool puerta_abierta;
    int pin_sensor;

public:

    SensorPuerta(int pin);

    bool conectar() override;

    bool esta_abierta();

    String leer_sensor() override;

    bool getEstadoPuerta();

};

#endif
