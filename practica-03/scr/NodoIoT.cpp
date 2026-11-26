#include "NodoIoT.h"

NodoIoT::NodoIoT(String id, int pinHardware)
    : idDispositivo(id),
      pin(pinHardware),
      conectado(false) {
}

void NodoIoT::iniciar() {
    pinMode(pin, INPUT);
    conectado = true;

    Serial.println(idDispositivo + " conectado");
}
