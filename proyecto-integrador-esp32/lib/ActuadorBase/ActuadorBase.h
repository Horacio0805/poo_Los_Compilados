#ifndef ACTUADOR_BASE_H
#define ACTUADOR_BASE_H

#include <Arduino.h>

class ActuadorBase {

protected:

    String id_dispositivo;
    String ubicacion;
    bool estado_activo;
    int potencia_pwm;

public:

    ActuadorBase(String id, String lugar);

    virtual bool conectar();

    virtual void activar() = 0;

    virtual void desactivar() = 0;

    virtual void enviar_telemetria();

    String getId();
    String getUbicacion();
    bool getEstado();
    int getPotencia();

    void setUbicacion(String lugar);
    bool setPotencia(int valor);

    virtual ~ActuadorBase();

};

#endif
