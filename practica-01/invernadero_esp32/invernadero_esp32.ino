/*
  invernadero_esp32.ino
  Practica 01 - Sistemas de Control (ESP32 / C++) - Version 2.0.0
  Micro-Invernadero Inteligente

  Pines (asignacion estricta segun la guia):
    GPIO 34  Sensor Termico (LM35/DHT)   ADC, 12 bits
    GPIO 32  Sensor LDR                  ADC, luz ambiental
    GPIO 18  Ventilador (Motor DC)       PWM (ledc), 5 kHz
    GPIO 19  LED de Potencia             PWM (ledc)

  Algoritmo de control:
    - Gestion termica : ventilador a 100% PWM si temperatura > 30 C
    - Gestion luminica : a menor LDR, mayor duty cycle del LED
                         (control proporcional inverso)

  Comunicacion serial:
    Acepta comandos de texto por el Monitor Serie y reporta
    telemetria en formato JSON simple.
    Comandos disponibles:
      leer            -> imprime telemetria actual (JSON)
      ventilador on   -> fuerza ventilador a 100% manualmente
      ventilador off  -> libera el forzado manual del ventilador
      led <0-255>     -> fuerza el LED a un duty cycle manual
      led auto        -> vuelve el LED a control proporcional automatico
      ayuda           -> lista de comandos
*/

// ---------------------------------------------------------------------
// Configuracion de pines
// ---------------------------------------------------------------------
const int PIN_SENSOR_TEMP = 34;   // ADC - LM35/DHT (termico)
const int PIN_SENSOR_LDR  = 32;   // ADC - luz ambiental
const int PIN_VENTILADOR  = 18;   // PWM - motor DC
const int PIN_LED         = 19;   // PWM - LED de potencia

// ---------------------------------------------------------------------
// Configuracion PWM (ledc)
// ---------------------------------------------------------------------
const int CANAL_VENTILADOR = 0;
const int CANAL_LED        = 1;
const int PWM_FREQ_HZ      = 5000;   // 5 kHz, segun requisito tecnico
const int PWM_RESOLUCION   = 8;      // 8 bits -> duty 0-255

// ---------------------------------------------------------------------
// Parametros del algoritmo de control
// ---------------------------------------------------------------------
const float TEMP_UMBRAL_C   = 30.0;   // Activa ventilador al 100% por encima de esto
const int   ADC_RESOLUCION  = 4096;   // 12 bits -> 0-4095
const float ADC_VREF        = 3.3;    // Voltaje de referencia del ADC

// LM35: 10 mV/°C. Ajustar segun el sensor real que se use.
const float LM35_MV_POR_C = 10.0;

// ---------------------------------------------------------------------
// Estado de control manual / automatico
// ---------------------------------------------------------------------
bool ventiladorForzadoManual = false;
bool ledEnAutomatico = true;
int  ledDutyManual = 0;

// ---------------------------------------------------------------------
// Lecturas actuales (para telemetria)
// ---------------------------------------------------------------------
float temperaturaC = 0.0;
int   lecturaLDR   = 0;
int   dutyVentilador = 0;
int   dutyLED         = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  // Resolucion del ADC a 12 bits, como exige la guia
  analogReadResolution(12);
  pinMode(PIN_SENSOR_TEMP, INPUT);
  pinMode(PIN_SENSOR_LDR, INPUT);

  // Configuracion obligatoria de PWM: ledcSetup + ledcAttachPin
  ledcSetup(CANAL_VENTILADOR, PWM_FREQ_HZ, PWM_RESOLUCION);
  ledcAttachPin(PIN_VENTILADOR, CANAL_VENTILADOR);

  ledcSetup(CANAL_LED, PWM_FREQ_HZ, PWM_RESOLUCION);
  ledcAttachPin(PIN_LED, CANAL_LED);

  ledcWrite(CANAL_VENTILADOR, 0);
  ledcWrite(CANAL_LED, 0);

  Serial.println("{\"status\":\"listo\",\"mensaje\":\"Invernadero ESP32 v2.0.0 inicializado\"}");
  imprimirAyuda();
}

void loop() {
  leerSensores();
  aplicarControl();
  procesarComandoSerial();
  delay(200);   // periodo de muestreo/control
}

// ---------------------------------------------------------------------
// Lectura de sensores
// ---------------------------------------------------------------------
void leerSensores() {
  int lecturaCruda = analogRead(PIN_SENSOR_TEMP);
  float voltajeMv = (lecturaCruda / (float)ADC_RESOLUCION) * ADC_VREF * 1000.0;
  temperaturaC = voltajeMv / LM35_MV_POR_C;

  lecturaLDR = analogRead(PIN_SENSOR_LDR);
}

// ---------------------------------------------------------------------
// Logica de control (termico + luminico)
// ---------------------------------------------------------------------
void aplicarControl() {
  // --- Gestion termica: ventilador ON/PWM 100% sobre el umbral ---
  if (ventiladorForzadoManual) {
    dutyVentilador = 255;
  } else if (temperaturaC > TEMP_UMBRAL_C) {
    dutyVentilador = 255;   // 100% duty cycle
  } else {
    dutyVentilador = 0;
  }
  ledcWrite(CANAL_VENTILADOR, dutyVentilador);

  // --- Gestion luminica: LED proporcional inverso al LDR ---
  if (ledEnAutomatico) {
    // A menor lectura de LDR (mas oscuridad), mayor duty cycle del LED.
    dutyLED = 255 - map(lecturaLDR, 0, ADC_RESOLUCION - 1, 0, 255);
    dutyLED = constrain(dutyLED, 0, 255);
  } else {
    dutyLED = ledDutyManual;
  }
  ledcWrite(CANAL_LED, dutyLED);
}

// ---------------------------------------------------------------------
// Comunicacion serial: comandos + telemetria JSON
// ---------------------------------------------------------------------
void procesarComandoSerial() {
  if (!Serial.available()) return;

  String linea = Serial.readStringUntil('\n');
  linea.trim();
  linea.toLowerCase();

  if (linea == "leer") {
    imprimirTelemetria();
  } else if (linea == "ventilador on") {
    ventiladorForzadoManual = true;
    Serial.println("{\"ok\":\"ventilador forzado a 100%\"}");
  } else if (linea == "ventilador off") {
    ventiladorForzadoManual = false;
    Serial.println("{\"ok\":\"ventilador vuelve a control automatico\"}");
  } else if (linea == "led auto") {
    ledEnAutomatico = true;
    Serial.println("{\"ok\":\"led vuelve a control automatico\"}");
  } else if (linea.startsWith("led ")) {
    int valor = linea.substring(4).toInt();
    valor = constrain(valor, 0, 255);
    ledDutyManual = valor;
    ledEnAutomatico = false;
    Serial.print("{\"ok\":\"led forzado a duty ");
    Serial.print(valor);
    Serial.println("\"}");
  } else if (linea == "ayuda") {
    imprimirAyuda();
  } else if (linea.length() > 0) {
    Serial.println("{\"error\":\"comando no reconocido, escriba 'ayuda'\"}");
  }
}

void imprimirTelemetria() {
  Serial.print("{");
  Serial.print("\"temperatura_c\":");
  Serial.print(temperaturaC, 2);
  Serial.print(",\"ldr_raw\":");
  Serial.print(lecturaLDR);
  Serial.print(",\"ventilador_duty\":");
  Serial.print(dutyVentilador);
  Serial.print(",\"led_duty\":");
  Serial.print(dutyLED);
  Serial.print(",\"ventilador_manual\":");
  Serial.print(ventiladorForzadoManual ? "true" : "false");
  Serial.print(",\"led_auto\":");
  Serial.print(ledEnAutomatico ? "true" : "false");
  Serial.println("}");
}

void imprimirAyuda() {
  Serial.println("Comandos disponibles:");
  Serial.println("  leer            -> telemetria actual (JSON)");
  Serial.println("  ventilador on   -> fuerza ventilador al 100%");
  Serial.println("  ventilador off  -> libera el forzado manual");
  Serial.println("  led <0-255>     -> fuerza duty cycle del LED");
  Serial.println("  led auto        -> control proporcional automatico");
  Serial.println("  ayuda           -> esta lista");
}
