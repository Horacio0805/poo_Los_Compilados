
#ifndef ACTUADOR_ALARMA_H
#define ACTUADOR_ALARMA_H

#include <Arduino.h>
#include "../ActuadorBase/ActuadorBase.h"

class ActuadorAlarma : public ActuadorBase {

private:

    int pin_buzzer;
    unsigned long duracion_alarma;
    unsigned long inicio_alarma;

    bool alarma_activa;

public:

    ActuadorAlarma(int pin, unsigned long duracion = 500);

    bool conectar() override;

    void activar() override;

    void desactivar() override;

    void emitir_alerta();

    void actualizar();

    bool getEstadoAlarma();

};

#endif
