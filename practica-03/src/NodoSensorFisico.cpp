#include "NodoSensorFisico.h"

NodoSensorFisico::NodoSensorFisico(
    String id, int pinHardware)
    : NodoIoT(id, pinHardware),
      _umbralAlerta(2048),
      _valorActual(0) {
}

void NodoSensorFisico::setUmbralAlerta(int umbral) {
    if (umbral >= 0 && umbral <= 4095) {
        _umbralAlerta = umbral;
    } else {
        Serial.println("Error: umbral fuera de rango");
    }
}

int NodoSensorFisico::getValorActual() const {
    return _valorActual;
}

bool NodoSensorFisico::hayAlerta() const {
    return _valorActual >= _umbralAlerta;
}

void NodoSensorFisico::iniciar() {
    NodoIoT::iniciar();
}

void NodoSensorFisico::ejecutarAccion() {
    _valorActual = analogRead(pin);

    Serial.print(idDispositivo);
    Serial.print(" | Lectura ADC: ");
    Serial.print(_valorActual);

    if (hayAlerta()) {
        Serial.println(" | ALERTA");
    } else {
        Serial.println(" | NORMAL");
    }
}
