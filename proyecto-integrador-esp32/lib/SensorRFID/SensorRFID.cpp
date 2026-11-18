
#include "SensorRFID.h"

// Pines del lector RFID
#define RFID_SS  5
#define RFID_RST 22

SensorRFID::SensorRFID(String uidAutorizado)
    : SensorBase("RFID01", "Entrada principal"),
      lector(RFID_SS, RFID_RST),
      uid_autorizado(uidAutorizado) {

    uid_detectado = "";

}

bool SensorRFID::conectar() {

    lector.PCD_Init();

    return SensorBase::conectar();

}

String SensorRFID::leer_tarjeta() {

    if (!lector.PICC_IsNewCardPresent()) {
        return "";
    }

    if (!lector.PICC_ReadCardSerial()) {
        return "";
    }

    uid_detectado = "";

    for (byte i = 0; i < lector.uid.size; i++) {

        if (i > 0) {
            uid_detectado += " ";
        }

        if (lector.uid.uidByte[i] < 0x10) {
            uid_detectado += "0";
        }

        uid_detectado += String(
            lector.uid.uidByte[i], HEX
        );
    }

    uid_detectado.toUpperCase();

    lector.PICC_HaltA();
    lector.PCD_StopCrypto1();

    return uid_detectado;

}

String SensorRFID::leer_sensor() {

    return uid_detectado;

}

bool SensorRFID::validar_uid(const String& uid) {

    return uid == uid_autorizado;

}
