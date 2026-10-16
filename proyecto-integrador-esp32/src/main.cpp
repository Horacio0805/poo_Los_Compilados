
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ============================================
// CONFIGURACION DE PINES
// ============================================
#define RFID_SS     5
#define RFID_RST    22
#define PUERTA_PIN  27
#define SERVO_PIN   25
#define BUZZER_PIN  26
#define LED_VERDE   32
#define LED_ROJO    33

// Cambiar por el UID de tu tarjeta autorizada
const String UID_AUTORIZADO = "A1 B2 C3 D4";

// ============================================
// 1. SUPERCLASE PADRE: NodoIoT
// ============================================
class NodoIoT {
protected:
  String id_dispositivo;
  String ip;
  String ubicacion;
  bool conectado;

public:
  NodoIoT(String id, String direccion, String lugar)
    : id_dispositivo(id), ip(direccion),
      ubicacion(lugar), conectado(false) {}

  virtual bool conectar() {
    conectado = true;
    return conectado;
  }

  virtual void enviar_telemetria() {
    Serial.println("Dispositivo: " + id_dispositivo);
  }

  virtual ~NodoIoT() {}
};

// ============================================
// 2. SUBCLASE: NodoSensor
// ============================================
class NodoSensor : public NodoIoT {
private:
  float limite_alerta;

public:
  NodoSensor(String id, String lugar)
    : NodoIoT(id, "Sin IP", lugar),
      limite_alerta(0) {}

  bool establecer_limite(float valor) {
    if (!isfinite(valor) || valor < 0 || valor > 100) {
      return false;
    }
    limite_alerta = valor;
    return true;
  }

  virtual String leer_sensor() {
    return "Sin lectura";
  }

  void enviar_telemetria() override {
    Serial.println("Sensor: " + id_dispositivo);
    Serial.println("Lectura: " + leer_sensor());
  }
};

// ============================================
// 3. SUBCLASE: NodoActuador
// ============================================
class NodoActuador : public NodoIoT {
private:
  bool estado_activo;
  int potencia_pwm;

public:
  NodoActuador(String id, String lugar)
    : NodoIoT(id, "Sin IP", lugar),
      estado_activo(false), potencia_pwm(0) {}

  virtual void activar() {
    estado_activo = true;
  }

  virtual void desactivar() {
    estado_activo = false;
  }

  bool establecer_potencia(int valor) {
    if (valor < 0 || valor > 255) return false;
    potencia_pwm = valor;
    return true;
  }

  void enviar_telemetria() override {
    Serial.print("Actuador: ");
    Serial.println(id_dispositivo);
    Serial.print("Estado: ");
    Serial.println(estado_activo ? "Activo" : "Inactivo");
  }
};

// ============================================
// 4. SENSOR RFID
// ============================================
class SensorRFID : public NodoSensor {
private:
  MFRC522 lector;
  String uid_detectado;

public:
  SensorRFID()
    : NodoSensor("RFID01", "Entrada principal"),
      lector(RFID_SS, RFID_RST) {}

  bool conectar() override {
    lector.PCD_Init();
    return NodoIoT::conectar();
  }

  String leer_tarjeta() {
    if (!lector.PICC_IsNewCardPresent()) return "";
    if (!lector.PICC_ReadCardSerial()) return "";

    uid_detectado = "";

    for (byte i = 0; i < lector.uid.size; i++) {
      if (i > 0) uid_detectado += " ";
      if (lector.uid.uidByte[i] < 0x10)
        uid_detectado += "0";

      uid_detectado += String(lector.uid.uidByte[i], HEX);
    }

    uid_detectado.toUpperCase();

    lector.PICC_HaltA();
    lector.PCD_StopCrypto1();

    return uid_detectado;
  }

  String leer_sensor() override {
    return uid_detectado;
  }

  bool validar_uid(String uid) {
    return uid == UID_AUTORIZADO;
  }
};

// ============================================
// 5. SENSOR MAGNETICO DE PUERTA
// ============================================
class SensorPuerta : public NodoSensor {
private:
  bool puerta_abierta;

public:
  SensorPuerta()
    : NodoSensor("PUERTA01", "Marco de puerta"),
      puerta_abierta(false) {}

  bool conectar() override {
    pinMode(PUERTA_PIN, INPUT_PULLUP);
    return NodoIoT::conectar();
  }

  bool esta_abierta() {
    // Contacto NC: cerrado = LOW
    puerta_abierta = digitalRead(PUERTA_PIN) == HIGH;
    return puerta_abierta;
  }

  String leer_sensor() override {
    return esta_abierta() ? "Abierta" : "Cerrada";
  }
};

// ============================================
// 6. ACTUADOR: CERRADURA CON SERVOMOTOR
// ============================================
class ActuadorCerradura : public NodoActuador {
private:
  Servo servo;
  int angulo_servo;

public:
  ActuadorCerradura()
    : NodoActuador("SERVO01", "Cerradura"),
      angulo_servo(0) {}

  bool conectar() override {
    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN, 500, 2400);
    cerrar_cerradura();
    return NodoIoT::conectar();
  }

  void abrir_cerradura() {
    angulo_servo = 90;
    servo.write(angulo_servo);
    NodoActuador::activar();
    Serial.println("Cerradura desbloqueada");
  }

  void cerrar_cerradura() {
    angulo_servo = 0;
    servo.write(angulo_servo);
    NodoActuador::desactivar();
    Serial.println("Cerradura bloqueada");
  }

  void activar() override {
    abrir_cerradura();
  }

  void desactivar() override {
    cerrar_cerradura();
  }
};

// ============================================
// 7. ACTUADOR: ALARMA BUZZER
// ============================================
class ActuadorAlarma : public NodoActuador {
private:
  int duracion_alarma;

public:
  ActuadorAlarma()
    : NodoActuador("ALARMA01", "Entrada"),
      duracion_alarma(500) {}

  bool conectar() override {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    return NodoIoT::conectar();
  }

  void activar() override {
    NodoActuador::activar();
    digitalWrite(BUZZER_PIN, HIGH);
  }

  void desactivar() override {
    digitalWrite(BUZZER_PIN, LOW);
    NodoActuador::desactivar();
  }

  void emitir_alerta() {
    activar();
    delay(duracion_alarma);
    desactivar();
  }
};

// ============================================
// CREACION DE OBJETOS
// ============================================
SensorRFID rfid;
SensorPuerta sensorPuerta;
ActuadorCerradura cerradura;
ActuadorAlarma alarma;

bool accesoActivo = false;
bool puertaFueAbierta = false;
unsigned long inicioAcceso = 0;

const unsigned long TIEMPO_ACCESO = 5000;
const unsigned long TIEMPO_MAX_ABIERTO = 15000;

// ============================================
// FUNCION SETUP
// ============================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO, LOW);

  SPI.begin(18, 19, 23, RFID_SS);

  rfid.conectar();
  sensorPuerta.conectar();
  cerradura.conectar();
  alarma.conectar();

  Serial.println("==============================");
  Serial.println("CONTROL DE ACCESO CON RFID");
  Serial.println("ESP32 - Sistema iniciado");
  Serial.println("==============================");
}

// ============================================
// BUCLE PRINCIPAL
// ============================================
void loop() {

  String uid = rfid.leer_tarjeta();

  // Revisar tarjeta presentada
  if (uid != "") {
    Serial.println("Tarjeta detectada: " + uid);

    if (rfid.validar_uid(uid)) {
      Serial.println("ACCESO AUTORIZADO");

      if (!accesoActivo) {
        cerradura.abrir_cerradura();
        digitalWrite(LED_VERDE, HIGH);
        digitalWrite(LED_ROJO, LOW);

        accesoActivo = true;
        puertaFueAbierta = false;
        inicioAcceso = millis();
      }

    } else {
      Serial.println("ACCESO DENEGADO");
      digitalWrite(LED_ROJO, HIGH);
      alarma.emitir_alerta();
      digitalWrite(LED_ROJO, LOW);
    }
  }

  // Supervisar apertura de puerta
  bool puertaAbierta = sensorPuerta.esta_abierta();

  if (accesoActivo) {

    if (puertaAbierta) {
      puertaFueAbierta = true;
    }

    // Bloquear solamente si ya abrieron
    // y despues cerraron la puerta
    if (puertaFueAbierta && !puertaAbierta) {
      cerradura.cerrar_cerradura();
      digitalWrite(LED_VERDE, LOW);
      accesoActivo = false;
    }

    // Tiempo de espera sin abrir la puerta
    else if (!puertaFueAbierta &&
             millis() - inicioAcceso >= TIEMPO_ACCESO) {
      cerradura.cerrar_cerradura();
      digitalWrite(LED_VERDE, LOW);
      accesoActivo = false;
    }

    // Alerta si permanece abierta demasiado tiempo
    else if (puertaAbierta &&
             millis() - inicioAcceso >= TIEMPO_MAX_ABIERTO) {
      Serial.println("ALERTA: Puerta abierta");
      // Solo mensaje para evitar alarma continua
    }
  }

  delay(100);
}
