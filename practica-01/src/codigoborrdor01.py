```python
import os
import random
import time
from datetime import datetime

# Sensores
temperatura = 25.0
presion = 1.0
caudal = 0.0

# Actuadores
bomba = 0.0
valvula = 0

# Control
setpoint_temp = 75.0
setpoint_presion = 8.0
modo = "MANUAL"
interlock = False

# Historial
eventos = []


def limpiar():
    os.system("cls" if os.name == "nt" else "clear")


def registrar_evento(mensaje):
    hora = datetime.now().strftime("%H:%M:%S")
    eventos.append(f"[{hora}] {mensaje}")

    if len(eventos) > 10:
        eventos.pop(0)


def seguridad():
    global bomba, valvula, interlock

    alarmas = []

    if temperatura >= 85:
        alarmas.append("ALTA TEMPERATURA")

    if presion >= 12:
        alarmas.append("ALTA PRESION")

    if alarmas:
        bomba = 100
        valvula = 1
        interlock = True

        registrar_evento(
            "INTERLOCK ACTIVADO: " + " / ".join(alarmas)
        )

        return True

    return False


def actualizar_proceso():
    global temperatura, presion, caudal

    # Modelo simplificado del reactor
    calentamiento = 1.2
    enfriamiento = bomba * 0.045

    temperatura += calentamiento - enfriamiento

    if valvula:
        presion -= 0.45
    else:
        presion += 0.15

    caudal = bomba * 0.8

    temperatura = max(0, temperatura)
    presion = max(0, presion)
    caudal = max(0, caudal)

    seguridad()


def control_automatico():
    global bomba, valvula

    if interlock:
        return

    error_temp = temperatura - setpoint_temp
    error_presion = presion - setpoint_presion

    if error_temp >= 10:
        bomba = 100
    elif error_temp >= 5:
        bomba = 70
    elif error_temp >= 2:
        bomba = 45
    else:
        bomba = 25

    if error_presion >= 2:
        valvula = 1
    else:
        valvula = 0


def mostrar():
    limpiar()

    estado = "INTERLOCK" if interlock else "NORMAL"

    print("=" * 50)
    print("              HMI - REACTOR")
    print("=" * 50)

    print(f"Modo:             {modo}")
    print(f"Estado:           {estado}")
    print("-" * 50)

    print(f"Temperatura:      {temperatura:6.1f} °C")
    print(f"Setpoint Temp.:   {setpoint_temp:6.1f} °C")
    print(f"Presion:          {presion:6.1f} Bar")
    print(f"Setpoint Pres.:   {setpoint_presion:6.1f} Bar")
    print(f"Caudal:           {caudal:6.1f} L/min")

    print("-" * 50)

    print(f"Bomba:            {bomba:6.1f} %")
    print(
        f"Valvula:          {'OPEN' if valvula else 'CLOSED'}"
    )

    print("-" * 50)

    if temperatura >= 85:
        print("ALARMA: ALTA TEMPERATURA")

    if presion >= 12:
        print("ALARMA: ALTA PRESION")

    if interlock:
        print("!!! INTERLOCK DE SEGURIDAD !!!")

    print("=" * 50)


def mostrar_eventos():
    limpiar()

    print("=" * 50)
    print("             HISTORIAL DE EVENTOS")
    print("=" * 50)

    if not eventos:
        print("No hay eventos registrados.")
    else:
        for evento in eventos:
            print(evento)

    print("=" * 50)
    input("\nPresiona ENTER para continuar...")


def configurar_setpoints():
    global setpoint_temp, setpoint_presion

    limpiar()

    print("===== CONFIGURACION =====")

    try:
        nueva_temp = float(
            input(f"Setpoint temperatura [{setpoint_temp}]: ")
        )

        nueva_presion = float(
            input(f"Setpoint presion [{setpoint_presion}]: ")
        )

        if 20 <= nueva_temp <= 85:
            setpoint_temp = nueva_temp
        else:
            print("Temperatura fuera de rango.")
            time.sleep(1)
            return

        if 1 <= nueva_presion <= 12:
            setpoint_presion = nueva_presion
        else:
            print("Presion fuera de rango.")
            time.sleep(1)
            return

        registrar_evento("Setpoints modificados")

    except ValueError:
        print("Entrada no valida.")
        time.sleep(1)


def modo_manual():
    global bomba, valvula, modo

    modo = "MANUAL"

    limpiar()

    print("===== CONTROL MANUAL =====")

    try:
        nueva_bomba = float(
            input("Bomba [0-100%]: ")
        )

        nueva_valvula = int(
            input("Valvula [0=CLOSE / 1=OPEN]: ")
        )

        if not 0 <= nueva_bomba <= 100:
            print("Valor de bomba fuera de rango.")
            time.sleep(1)
            return

        if nueva_valvula not in (0, 1):
            print("Valor de valvula no valido.")
            time.sleep(1)
            return

        bomba = nueva_bomba
        valvula = nueva_valvula

        registrar_evento("Control manual modificado")

        if seguridad():
            print("\nINTERLOCK ACTIVADO")

        time.sleep(1)

    except ValueError:
        print("Entrada no valida.")
        time.sleep(1)


def ejecutar_automatico():
    global modo

    modo = "AUTOMATICO"

    if interlock:
        print("\nEl reactor esta bloqueado.")
        input("ENTER para continuar...")
        return

    control_automatico()
    actualizar_proceso()

    time.sleep(1)


def pruebas():
    global temperatura, presion, interlock, modo

    modo = "PRUEBAS"

    limpiar()

    print("===== PRUEBA DE SEGURIDAD =====")

    temperatura = random.uniform(85, 100)
    presion = random.uniform(12, 15)

    interlock = False
    seguridad()

    print(f"\nTemperatura simulada: {temperatura:.1f} °C")
    print(f"Presion simulada:     {presion:.1f} Bar")

    if interlock:
        print("\nINTERLOCK ACTIVADO CORRECTAMENTE")

    input("\nENTER para continuar...")


def resetear():
    global temperatura, presion, caudal
    global bomba, valvula, interlock, modo

    temperatura = 25.0
    presion = 1.0
    caudal = 0.0

    bomba = 0.0
    valvula = 0

    interlock = False
    modo = "MANUAL"

    registrar_evento("Sistema reiniciado")


def menu():
    print("\n1. Control manual")
    print("2. Control automatico")
    print("3. Prueba de seguridad")
    print("4. Configurar setpoints")
    print("5. Leer caudal")
    print("6. Leer manometro")
    print("7. Historial de eventos")
    print("8. Resetear sistema")
    print("9. Salir")


while True:

    mostrar()
    menu()

    opcion = input("\nSelecciona una opcion: ")

    if opcion == "1":

        modo_manual()

    elif opcion == "2":

        ejecutar_automatico()

    elif opcion == "3":

        pruebas()

    elif opcion == "4":

        configurar_setpoints()

    elif opcion == "5":

        print(f"\nCaudal actual: {caudal:.1f} L/min")
        input("ENTER para continuar...")

    elif opcion == "6":

        print(f"\nPresion actual: {presion:.1f} Bar")
        input("ENTER para continuar...")

    elif opcion == "7":

        mostrar_eventos()

    elif opcion == "8":

        resetear()
        time.sleep(1)

    elif opcion == "9":

        limpiar()
        print("Simulador terminado.")
        break

    else:

        print("\nOpcion no valida.")
        time.sleep(1)
```
