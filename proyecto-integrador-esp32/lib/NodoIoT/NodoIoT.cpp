
#include "NodoIoT.h"

NodoIoT::NodoIoT(
    String id,
    String direccion,
    String lugar
) {

    id_dispositivo = id;
    ip = direccion;
    ubicacion = lugar;
    conectado = false;

}

bool NodoIoT::conectar() {

    conectado = true;

    return conectado;

}

void NodoIoT::enviar_telemetria() {

    Serial.println("======================");

    Serial.println("DISPOSITIVO IoT");

    Serial.println("ID: " + id_dispositivo);

    Serial.println("IP: " + ip);

    Serial.println("Ubicacion: " + ubicacion);

    Serial.println("Conexion: " +
        String(conectado ? "Activa" : "Inactiva"));

    Serial.println("======================");

}

String NodoIoT::getId() {

    return id_dispositivo;

}

String NodoIoT::getIP() {

    return ip;

}

String NodoIoT::getUbicacion() {

    return ubicacion;

}

bool NodoIoT::getEstadoConexion() {

    return conectado;

}

void NodoIoT::setIP(String direccion) {

    ip = direccion;

}

void NodoIoT::setUbicacion(String lugar) {

    ubicacion = lugar;

}

NodoIoT::~NodoIoT() {}

