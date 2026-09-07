/*
  invernadero_esp32.ino
  Practica 01 - Sistemas de Control (ESP32) - Version 2.0.0

  Control PID de temperatura para invernadero:
    - Sensor: DS18B20 (OneWire, digital, resistente a humedad)
    - Actuador: elemento calefactor conmutado por MOSFET/SSR vía PWM (ledc)
    - Controlador: PID discreto (librería PID_v1) portado del simulador
      de Python (practica-01/src/reactor_sim.py)

  Librerías requeridas (Arduino Library Manager):
    - OneWire
    - DallasTemperature
    - PID (br3ttb / PID_v1)

  Conexiones sugeridas:
    - DS18B20 DATA  -> GPIO4  (con resistencia pull-up de 4.7k a 3V3)
    - Gate MOSFET   -> GPIO25 (salida PWM al calentador)
    - GND común entre ESP32, sensor y etapa de potencia

  Comandos por Monitor Serial (115200 baud):
    SP:<valor>   -> cambia el setpoint, ej. "SP:28.5"
    KP:<valor>   -> cambia Kp en caliente
    KI:<valor>   -> cambia Ki en caliente
    KD:<valor>   -> cambia Kd en caliente
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

// ---------------- Configuración de hardware ----------------
constexpr uint8_t PIN_DS18B20   = 4;
constexpr uint8_t PIN_HEATER    = 25;
constexpr uint8_t PWM_CHANNEL   = 0;
constexpr uint32_t PWM_FREQ_HZ  = 5000;
constexpr uint8_t PWM_RES_BITS  = 8;   // 0-255

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensores(&oneWire);

// ---------------- Variables del PID ----------------
double temperaturaActual = 25.0;
double salidaPID         = 0.0;   // 0-255 (mapea a 0-100% de potencia)
double setpoint           = 28.0; // °C, valor por defecto para invernadero

double Kp = 8.0, Ki = 0.35, Kd = 4.0;  // ganancias iniciales, ajustar en campo
PID controladorPID(&temperaturaActual, &salidaPID, &setpoint, Kp, Ki, Kd, DIRECT);

// ---------------- Temporización ----------------
constexpr unsigned long PERIODO_MUESTREO_MS = 500;
unsigned long ultimoMuestreo = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  sensores.begin();
  sensores.setResolution(12);

  ledcSetup(PWM_CHANNEL, PWM_FREQ_HZ, PWM_RES_BITS);
  ledcAttachPin(PIN_HEATER, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);

  controladorPID.SetMode(AUTOMATIC);
  controladorPID.SetOutputLimits(0, 255);
  controladorPID.SetSampleTime(PERIODO_MUESTREO_MS);

  Serial.println(F("=== Invernadero ESP32 - Control PID v2.0.0 ==="));
  Serial.println(F("Comandos: SP:<valor>  KP:<valor>  KI:<valor>  KD:<valor>"));
}

void procesarComandoSerial() {
  if (!Serial.available()) return;

  String linea = Serial.readStringUntil('\n');
  linea.trim();
  if (linea.length() < 4) return;

  String prefijo = linea.substring(0, 3);
  float valor = linea.substring(3).toFloat();

  if (prefijo == "SP:") {
    setpoint = valor;
    Serial.print(F("Nuevo setpoint: "));
    Serial.println(setpoint);
  } else if (prefijo == "KP:") {
    Kp = valor;
    controladorPID.SetTunings(Kp, Ki, Kd);
    Serial.print(F("Nuevo Kp: "));
    Serial.println(Kp);
  } else if (prefijo == "KI:") {
    Ki = valor;
    controladorPID.SetTunings(Kp, Ki, Kd);
    Serial.print(F("Nuevo Ki: "));
    Serial.println(Ki);
  } else if (prefijo == "KD:") {
    Kd = valor;
    controladorPID.SetTunings(Kp, Ki, Kd);
    Serial.print(F("Nuevo Kd: "));
    Serial.println(Kd);
  }
}

void loop() {
  procesarComandoSerial();

  unsigned long ahora = millis();
  if (ahora - ultimoMuestreo >= PERIODO_MUESTREO_MS) {
    ultimoMuestreo = ahora;

    sensores.requestTemperatures();
    float lectura = sensores.getTempCByIndex(0);

    // DS18B20 devuelve -127 si hay error de lectura/cableado
    if (lectura > -100.0) {
      temperaturaActual = lectura;
    }

    controladorPID.Compute();
    ledcWrite(PWM_CHANNEL, (uint32_t)salidaPID);

    float porcentajePotencia = (salidaPID / 255.0) * 100.0;
    Serial.print(F("T="));
    Serial.print(temperaturaActual, 2);
    Serial.print(F("C  SP="));
    Serial.print(setpoint, 2);
    Serial.print(F("C  Salida="));
    Serial.print(porcentajePotencia, 1);
    Serial.println(F("%"));
  }
}
