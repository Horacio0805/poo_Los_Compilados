import os
import random
import time

# Sensores
temperatura = 25.0
presion = 1.0

# Actuadores
bomba = 0.0
valvula = 0


def limpiar():
    os.system("cls" if os.name == "nt" else "clear")


def seguridad():
    global bomba, valvula

    if temperatura > 85 or presion > 12:
        bomba = 100
        valvula = 1
        return True

    return False


def mostrar():
    limpiar()

    print("===== HMI REACTOR =====")
    print(f"Temperatura: {temperatura:.1f} °C")
    print(f"Presion:     {presion:.1f} Bar")
    print(f"Bomba:       {bomba:.0f} %")
    print(f"Valvula:     {'OPEN' if valvula else 'CLOSE'}")

    if seguridad():
        print("\n!!! INTERLOCK DE SEGURIDAD !!!")


def automatico():
    global temperatura, presion, bomba, valvula

    if seguridad():
        return

    if temperatura > 80:
        bomba = 100
    elif temperatura > 70:
        bomba = 60
    else:
        bomba = 20

    # Formula del reactor
    deltaT = 1.5 - (0.05 * bomba)
    temperatura += deltaT

    # Control de presion
    if presion > 10:
        valvula = 1
        presion -= 0.5
    else:
        valvula = 0
        presion += 0.2

    seguridad()


def pruebas():
    global temperatura, presion

    # Inyeccion de fallos
    temperatura = random.uniform(85, 100)
    presion = random.uniform(12, 15)

    seguridad()


while True:

    mostrar()

    print("\n1. Manual")
    print("2. Automatico")
    print("3. Pruebas")
    print("4. Leer caudal")
    print("5. Leer manometro")
    print("6. Salir")

    opcion = input("\nOpcion: ")

    if opcion == "1":

        bomba = float(input("Bomba 0-100%: "))
        valvula = int(input("Valvula 0=CLOSE, 1=OPEN: "))

        seguridad()

    elif opcion == "2":

        automatico()
        time.sleep(1)

    elif opcion == "3":

        pruebas()
        time.sleep(1)

    elif opcion == "4":

        print(f"\nCaudal: {bomba:.0f}%")
        input("ENTER para continuar...")

    elif opcion == "5":

        print(f"\nManometro: {presion:.1f} Bar")
        input("ENTER para continuar...")

    elif opcion == "6":

        print("Simulador terminado.")
        break

    else:

        print("Opcion no valida.")
        time.sleep(1)