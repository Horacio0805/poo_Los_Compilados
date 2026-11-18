
#ifndef NODO_IOT_H
#define NODO_IOT_H

#include <Arduino.h>

class NodoIoT {

protected:

    String id_dispositivo;
    String ip;
    String ubicacion;
    bool conectado;

public:

    NodoIoT(String id, String direccion, String lugar);

    virtual bool conectar();

    virtual void enviar_telemetria();

    String getId();
    String getIP();
    String getUbicacion();
    bool getEstadoConexion();

    void setIP(String direccion);
    void setUbicacion(String lugar);

    virtual ~NodoIoT();

};

#endif
