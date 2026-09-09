/*
 * Micro-Invernadero Inteligente
 * Practica 01 - Sistemas de Control
 * ESP32 / C++ - v2.1.0
 *
 * Control:
 *  - Temperatura > 30 °C -> Ventilador 100 %
 *  - Menor iluminación -> Mayor intensidad del LED
 *
 * Comunicación:
 *  - leer
 *  - ventilador on/off
 *  - led <0-255>
 *  - led auto
 *  - ayuda
 */

// ===================== PINES =====================
#define TEMP_PIN     34
#define LDR_PIN      32
#define FAN_PIN      18
#define LED_PIN      19

// ===================== PWM =======================
#define FAN_CH       0
#define LED_CH       1
#define PWM_FREQ     5000
#define PWM_RES      8

// ===================== CONTROL ===================
#define TEMP_LIMIT   30.0
#define ADC_MAX      4095.0
#define VREF         3.3
#define LM35_SCALE   100.0   // 10 mV/°C -> 100 °C/V

// ===================== ESTADO ====================
float temperatura = 0;
int ldr = 0;
int fanDuty = 0;
int ledDuty = 0;

bool fanManual = false;
bool ledAuto = true;
int ledManual = 0;


// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);

  analogReadResolution(12);

  pinMode(TEMP_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  ledcSetup(FAN_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(FAN_PIN, FAN_CH);

  ledcSetup(LED_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED_PIN, LED_CH);

  ledcWrite(FAN_CH, 0);
  ledcWrite(LED_CH, 0);

  Serial.println("\n=== MICRO-INVERNADERO INTELIGENTE ===");
  Serial.println("Sistema iniciado correctamente.");
  ayuda();
}


// ===================== LOOP ======================
void loop() {
  leerSensores();
  controlarSistema();
  comandos();

  delay(200);
}


// ===================== SENSORES ==================
void leerSensores() {
  int adcTemp = analogRead(TEMP_PIN);

  temperatura = (adcTemp / ADC_MAX) * VREF * LM35_SCALE;
  ldr = analogRead(LDR_PIN);
}


// ===================== CONTROL ===================
void controlarSistema() {

  // Control de temperatura
  fanDuty = (fanManual || temperatura > TEMP_LIMIT) ? 255 : 0;
  ledcWrite(FAN_CH, fanDuty);

  // Control de iluminación
  if (ledAuto) {
    ledDuty = map(ldr, 0, 4095, 255, 0);
  } else {
    ledDuty = ledManual;
  }

  ledDuty = constrain(ledDuty, 0, 255);
  ledcWrite(LED_CH, ledDuty);
}


// ===================== COMANDOS ==================
void comandos() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "leer") {
    telemetria();

  } else if (cmd == "ventilador on") {
    fanManual = true;
    Serial.println("{\"ok\":\"ventilador_manual_100\"}");

  } else if (cmd == "ventilador off") {
    fanManual = false;
    Serial.println("{\"ok\":\"ventilador_automatico\"}");

  } else if (cmd == "led auto") {
    ledAuto = true;
    Serial.println("{\"ok\":\"led_automatico\"}");

  } else if (cmd.startsWith("led ")) {
    ledManual = constrain(cmd.substring(4).toInt(), 0, 255);
    ledAuto = false;

    Serial.print("{\"ok\":\"led_manual\",\"duty\":");
    Serial.print(ledManual);
    Serial.println("}");

  } else if (cmd == "ayuda") {
    ayuda();

  } else if (cmd.length()) {
    Serial.println("{\"error\":\"comando_no_reconocido\"}");
  }
}


// ===================== TELEMETRÍA ===============
void telemetria() {
  Serial.print("{");
  Serial.print("\"temperatura\":");
  Serial.print(temperatura, 2);
  Serial.print(",\"ldr\":");
  Serial.print(ldr);
  Serial.print(",\"ventilador\":");
  Serial.print(fanDuty);
  Serial.print(",\"led\":");
  Serial.print(ledDuty);
  Serial.print(",\"fan_manual\":");
  Serial.print(fanManual ? "true" : "false");
  Serial.print(",\"led_auto\":");
  Serial.print(ledAuto ? "true" : "false");
  Serial.println("}");
}


// ===================== AYUDA =====================
void ayuda() {
  Serial.println("\nComandos:");
  Serial.println("  leer");
  Serial.println("  ventilador on");
  Serial.println("  ventilador off");
  Serial.println("  led <0-255>");
  Serial.println("  led auto");
  Serial.println("  ayuda\n");
}
