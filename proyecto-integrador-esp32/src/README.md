
# Sistema de Control de Acceso con RFID y ESP32

## Descripción del proyecto

Este proyecto consiste en el diseño de un sistema de control de acceso automatizado mediante tecnología RFID, utilizando un microcontrolador ESP32.

Su objetivo es identificar tarjetas o llaveros RFID autorizados y controlar un mecanismo de apertura mediante un servomotor.

El sistema también incorpora un sensor magnético para supervisar el estado de la puerta, indicadores LED y un buzzer para emitir alertas.

## Objetivo general

Diseñar un sistema de control de acceso mediante RFID y ESP32, aplicando los principios de programación orientada a objetos para administrar sensores y actuadores.

## Componentes utilizados

- ESP32 DevKit
- Módulo RFID RC522
- Sensor magnético MC-38
- Servomotor SG90
- Buzzer
- LED verde
- LED rojo
- Resistencias de 220 ohms
- Protoboard y cables Dupont
- Fuente regulada de 5 V para el servomotor

## Funcionamiento del sistema

1. El usuario acerca una tarjeta RFID al lector.
2. El ESP32 obtiene el identificador de la tarjeta.
3. El sistema verifica si está autorizada.
4. Si es válida, activa el servomotor y el LED verde.
5. Si no está autorizada, activa el buzzer y el LED rojo.
6. El sensor magnético supervisa el estado de la puerta.
7. El sistema vuelve a bloquear el mecanismo según las condiciones programadas.

## Arquitectura del software

El proyecto utiliza programación orientada a objetos.

### Superclase
- NodoIoT

### Clases derivadas
- NodoSensor
- NodoActuador

### Clases especializadas
- SensorRFID
- SensorPuerta
- ActuadorCerradura
- ActuadorAlarma

Se aplican los principios de herencia, encapsulamiento y polimorfismo.

## Estructura del repositorio

proyecto-integrador-esp32/
- docs/
  - reporte_etapa1.pdf
- diagrams/
  - uml_clases_inicial.png
  - diagrama_bloques.png
  - esquematico_conexion.png
- src/
  - main.cpp
- README.md

## Tecnologías utilizadas

- C++
- Arduino IDE
- ESP32
- RFID
- SPI
- Programación orientada a objetos
- GitHub

## Estado del proyecto

Etapa 1: Planteamiento, propuesta y arquitectura.

El código corresponde a una primera versión del prototipo. La integración física y las pruebas de funcionamiento se realizarán en las siguientes etapas.
