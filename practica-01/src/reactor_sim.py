import os
import time
import random

# Variables iniciales
temperatura = 25.0
presion = 1.0
bomba = 0
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
    print("===== SIMULADOR DE REACTOR =====")
    print(f"Temperatura: {temperatura:.1f} °C")
    print(f"Presion:     {presion:.1f} Bar")
    print(f"Bomba:       {bomba}%")
    print(f"Valvula:     {'ABIERTA' if valvula else 'CERRADA'}")

    if seguridad():
        print("\n!!! INTERLOCK DE SEGURIDAD !!!")


def automatico():
    global bomba, valvula, temperatura, presion

    if seguridad():
        return

    # Control sencillo de temperatura
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
    else:
        valvula = 0


def pruebas():
    global temperatura, presion

    temperatura = random.uniform(80, 100)
    presion = random.uniform(10, 14)


# Programa principal
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

        bomba = float(input("Bomba (0-100%): "))

        v = input("Valvula (0=cerrada, 1=abierta): ")
        valvula = int(v)

        seguridad()

    elif opcion == "2":

        automatico()
        time.sleep(2)

    elif opcion == "3":

        pruebas()
        seguridad()
        time.sleep(2)

    elif opcion == "4":

        print(f"\nCaudal: {bomba}%")
        input("ENTER para continuar...")

    elif opcion == "5":

        print(f"\nManometro: {presion:.2f} Bar")
        input("ENTER para continuar...")

    elif opcion == "6":

        print("Simulador terminado.")
        break

    else:

        print("Opcion no valida.")
        time.sleep(1)
