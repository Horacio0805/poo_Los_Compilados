
#ifndef SENSOR_RFID_H
#define SENSOR_RFID_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "../SensorBase/SensorBase.h"

class SensorRFID : public SensorBase {

private:

    MFRC522 lector;
    String uid_detectado;
    String uid_autorizado;

public:

    SensorRFID(String uidAutorizado);

    bool conectar() override;

    String leer_tarjeta();

    String leer_sensor() override;

    bool validar_uid(const String& uid);

};

#endif
