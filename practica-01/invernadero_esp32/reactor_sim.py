#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

#define LDR_PIN 34
#define LED_PIN 2

#define ENA 26
#define IN1 27
#define IN2 14

DHT dht(DHTPIN, DHTTYPE);

bool sistemaEncendido = false;

int temperaturaObjetivo = 30;

int ldrOscuro = 0;
int ldrLuz = 3109;

const int pwmFreq = 5000;
const int pwmResolution = 8;

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(LDR_PIN, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ledcAttach(LED_PIN, pwmFreq, pwmResolution);
  ledcAttach(ENA, pwmFreq, pwmResolution);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  ledcWrite(LED_PIN, 0);
  ledcWrite(ENA, 0);

  Serial.println();
  Serial.println("====================================");
  Serial.println("       SISTEMA DE CONTROL ESP32");
  Serial.println("====================================");
  Serial.println("Comandos:");
  Serial.println("encender");
  Serial.println("apagar");
  Serial.println("leer");
  Serial.println("ajustar 30");
  Serial.println();
  Serial.println("Temperatura permitida: 0 a 50 C");
  Serial.println("====================================");
}

void loop() {

  revisarComandos();

  if (sistemaEncendido) {

    controlarTemperatura();
    controlarIluminacion();

  } else {

    detenerVentilador();
    ledcWrite(LED_PIN, 0);
  }

  delay(200);
}

void revisarComandos() {

  if (Serial.available() > 0) {

    String comando = Serial.readStringUntil('\n');

    comando.trim();
    comando.toLowerCase();

    if (comando == "encender") {

      sistemaEncendido = true;

      Serial.println();
      Serial.println("Sistema ENCENDIDO");

    }

    else if (comando == "apagar") {

      sistemaEncendido = false;

      detenerVentilador();
      ledcWrite(LED_PIN, 0);

      Serial.println();
      Serial.println("Sistema APAGADO");

    }

    else if (comando == "leer") {

      mostrarDatos();

    }

    else if (comando.startsWith("ajustar")) {

      int espacio = comando.indexOf(' ');

      if (espacio != -1) {

        int nuevaTemperatura =
          comando.substring(espacio + 1).toInt();

        if (nuevaTemperatura >= 0 &&
            nuevaTemperatura <= 50) {

          temperaturaObjetivo = nuevaTemperatura;

          Serial.print("Temperatura objetivo: ");
          Serial.print(temperaturaObjetivo);
          Serial.println(" C");

        } else {

          Serial.println(
            "Error: usa un valor entre 0 y 50 C"
          );
        }

      } else {

        Serial.println("Ejemplo: ajustar 30");
      }
    }

    else {

      Serial.println("Comando no reconocido");
    }
  }
}

void controlarTemperatura() {

  float temperatura = dht.readTemperature();

  if (isnan(temperatura)) {

    Serial.println("Error al leer DHT11");

    detenerVentilador();

    return;
  }

  int pwmVentilador = 0;

  if (temperatura <= temperaturaObjetivo) {

    pwmVentilador = 0;

  }

  else if (temperatura >= 50) {

    pwmVentilador = 255;

  }

  else {

    pwmVentilador = map(
      temperatura * 10,
      temperaturaObjetivo * 10,
      500,
      120,
      255
    );

    pwmVentilador = constrain(
      pwmVentilador,
      120,
      255
    );
  }

  if (pwmVentilador > 0) {

    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    ledcWrite(
      ENA,
      pwmVentilador
    );

  } else {

    detenerVentilador();
  }
}

void controlarIluminacion() {

  int valorLDR = analogRead(LDR_PIN);

  int pwmLED = map(
    valorLDR,
    ldrOscuro,
    ldrLuz,
    255,
    0
  );

  pwmLED = constrain(
    pwmLED,
    0,
    255
  );

  ledcWrite(
    LED_PIN,
    pwmLED
  );
}

void detenerVentilador() {

  ledcWrite(ENA, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void mostrarDatos() {

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  int valorLDR = analogRead(LDR_PIN);

  if (isnan(temperatura) ||
      isnan(humedad)) {

    Serial.println("Error al leer DHT11");

    return;
  }

  int pwmLED = 0;
  int pwmVentilador = 0;

  if (sistemaEncendido) {

    pwmLED = map(
      valorLDR,
      ldrOscuro,
      ldrLuz,
      255,
      0
    );

    pwmLED = constrain(
      pwmLED,
      0,
      255
    );

    if (temperatura > temperaturaObjetivo) {

      if (temperatura >= 50) {

        pwmVentilador = 255;

      } else {

        pwmVentilador = map(
          temperatura * 10,
          temperaturaObjetivo * 10,
          500,
          120,
          255
        );

        pwmVentilador = constrain(
          pwmVentilador,
          120,
          255
        );
      }
    }
  }

  int porcentajeLED =
    map(
      pwmLED,
      0,
      255,
      0,
      100
    );

  int porcentajeVentilador =
    map(
      pwmVentilador,
      0,
      255,
      0,
      100
    );

  Serial.println();
  Serial.println("========== DATOS ==========");

  Serial.print("Sistema: ");

  if (sistemaEncendido) {

    Serial.println("ENCENDIDO");

  } else {

    Serial.println("APAGADO");
  }

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.print("Temperatura objetivo: ");
  Serial.print(temperaturaObjetivo);
  Serial.println(" C");

  Serial.print("LDR: ");
  Serial.println(valorLDR);

  Serial.print("LED: ");
  Serial.print(porcentajeLED);
  Serial.println(" %");

  Serial.print("Ventilador: ");
  Serial.print(porcentajeVentilador);
  Serial.println(" %");

  Serial.println("===========================");
}
