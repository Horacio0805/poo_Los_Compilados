#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

#include <Arduino.h>

class SensorBase {

protected:

    String id_dispositivo;
    String ubicacion;
    float limite_alerta;
    bool conectado;

public:

    SensorBase(String id, String lugar);

    virtual bool conectar();

    virtual String leer_sensor() = 0;

    virtual void enviar_telemetria();

    String getId();
    String getUbicacion();
    float getLimiteAlerta();
    bool getEstadoConexion();

    void setUbicacion(String lugar);
    bool setLimiteAlerta(float valor);

    virtual ~SensorBase();
};

#endif
