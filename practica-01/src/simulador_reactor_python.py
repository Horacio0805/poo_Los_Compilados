import os
import time

TEMP_MIN = 0.0
TEMP_MAX = 150.0
PRES_MIN = 0.0
PRES_MAX = 15.0

TEMP_INTERLOCK = 85.0
PRES_INTERLOCK = 12.0


def limpiar_pantalla():
    os.system("cls" if os.name == "nt" else "clear")


def limitar(valor, minimo, maximo):
    return max(minimo, min(valor, maximo))


class Reactor:
    def __init__(self):
        self.temperatura = 25.0
        self.presion = 1.0
        self.bomba = 0.0
        self.valvula = 0
        self.modo = "manual"
        self.interlock = False
        self.ciclo = 0

    def leer_temperatura(self):
        return self.temperatura

    def leer_presion(self):
        return self.presion

    def leer_caudal(self):
        return self.bomba

    def leer_manometro(self):
        return self.presion

    def revisar_interlocks(self):
        if self.temperatura > TEMP_INTERLOCK or self.presion > PRES_INTERLOCK:
            self.interlock = True
            self.bomba = 100.0
            self.valvula = 1
        else:
            self.interlock = False

    def actualizar_proceso(self):
        self.revisar_interlocks()

        delta_t = 1.5 - (0.05 * self.bomba)
        self.temperatura += delta_t

        incremento_termico = max(0.0, self.temperatura - 25.0) * 0.002
        delta_p = 0.03 + incremento_termico

        if self.valvula == 1:
            delta_p -= 0.60

        self.presion += delta_p

        self.temperatura = limitar(
            self.temperatura,
            TEMP_MIN,
            TEMP_MAX
        )

        self.presion = limitar(
            self.presion,
            PRES_MIN,
            PRES_MAX
        )

        self.ciclo += 1

        self.revisar_interlocks()

    def control_automatico(self):
        self.revisar_interlocks()

        if self.interlock:
            return

        if self.temperatura < 60.0:
            self.bomba = 0.0
        elif self.temperatura < 70.0:
            self.bomba = 30.0
        elif self.temperatura < 80.0:
            self.bomba = 60.0
        else:
            self.bomba = 90.0

        if self.presion > 10.0:
            self.valvula = 1
        elif self.presion < 9.0:
            self.valvula = 0

    def inyectar_fallo(self, opcion):
        if opcion == "1":
            self.temperatura = 90.0

        elif opcion == "2":
            self.presion = 13.0

        elif opcion == "3":
            self.temperatura = 95.0
            self.presion = 13.5

        self.revisar_interlocks()


def mostrar_hmi(reactor):
    limpiar_pantalla()

    print("=" * 55)
    print("       SIMULADOR DE REACTOR QUIMICO v1.0.0")
    print("=" * 55)

    print(f"Modo:              {reactor.modo.upper()}")
    print(f"Ciclo:             {reactor.ciclo}")

    print("-" * 55)

    print(f"Temperatura:       {reactor.temperatura:6.2f} °C")
    print(f"Presion:           {reactor.presion:6.2f} bar")
    print(f"Bomba:             {reactor.bomba:6.1f} %")

    if reactor.valvula:
        print("Valvula de alivio: ABIERTA")
    else:
        print("Valvula de alivio: CERRADA")

    print("-" * 55)

    if reactor.interlock:
        print("!!! INTERLOCK DE SEGURIDAD ACTIVO !!!")
        print("Bomba al 100 % y valvula ABIERTA.")
    else:
        print("Estado de seguridad: NORMAL")

    print("=" * 55)


def modo_manual(reactor):
    reactor.modo = "manual"

    while True:
        reactor.revisar_interlocks()

        mostrar_hmi(reactor)

        print("\nComandos:")
        print("bomba <0-100>")
        print("valvula <0|1>")
        print("leer caudal")
        print("leer manometro")
        print("paso")
        print("volver")
        print("salir")

        comando = input("\n> ").strip().lower()

        if comando.startswith("bomba "):

            if reactor.interlock:
                input("Interlock activo. Orden ignorada. Enter...")
                continue

            try:
                valor = float(comando.split()[1])

                if 0 <= valor <= 100:
                    reactor.bomba = valor
                else:
                    input("Valor fuera de rango. Enter...")

            except:
                input("Uso correcto: bomba <0-100>. Enter...")

        elif comando.startswith("valvula "):

            if reactor.interlock:
                input("Interlock activo. Orden ignorada. Enter...")
                continue

            try:
                valor = int(comando.split()[1])

                if valor == 0 or valor == 1:
                    reactor.valvula = valor
                else:
                    input("Use 0 o 1. Enter...")

            except:
                input("Uso correcto: valvula <0|1>. Enter...")

        elif comando == "leer caudal":

            input(
                f"Caudal: {reactor.leer_caudal():.1f} %. Enter..."
            )

        elif comando == "leer manometro":

            input(
                f"Manometro: {reactor.leer_manometro():.2f} bar. Enter..."
            )

        elif comando == "paso":

            reactor.actualizar_proceso()

        elif comando == "volver":

            return

        elif comando == "salir":

            raise SystemExit

        else:

            input("Comando no reconocido. Enter...")


def modo_automatico(reactor):
    reactor.modo = "automatico"

    while True:

        reactor.control_automatico()

        mostrar_hmi(reactor)

        print("\nModo automatico")
        print("Enter = ejecutar 1 ciclo")
        print("auto <n> = ejecutar varios ciclos")
        print("leer caudal")
        print("leer manometro")
        print("volver")
        print("salir")

        comando = input("\n> ").strip().lower()

        if comando == "":

            reactor.control_automatico()
            reactor.actualizar_proceso()

        elif comando.startswith("auto "):

            try:

                n = int(comando.split()[1])

                for i in range(n):

                    reactor.control_automatico()
                    reactor.actualizar_proceso()

                    mostrar_hmi(reactor)

                    time.sleep(0.1)

            except:

                input("Uso correcto: auto <n>. Enter...")

        elif comando == "leer caudal":

            input(
                f"Caudal: {reactor.leer_caudal():.1f} %. Enter..."
            )

        elif comando == "leer manometro":

            input(
                f"Manometro: {reactor.leer_manometro():.2f} bar. Enter..."
            )

        elif comando == "volver":

            return

        elif comando == "salir":

            raise SystemExit

        else:

            input("Comando no reconocido. Enter...")


def modo_pruebas(reactor):
    reactor.modo = "pruebas"

    while True:

        mostrar_hmi(reactor)

        print("\nINYECCION DE FALLOS")

        print("1. Sobretemperatura 90 °C")
        print("2. Sobrepresion 13 bar")
        print("3. Temperatura y presion altas")
        print("4. Restaurar condiciones")
        print("5. Ejecutar un paso")
        print("6. Volver")

        opcion = input("\nSeleccione: ").strip()

        if opcion in ("1", "2", "3"):

            reactor.inyectar_fallo(opcion)

            input("Fallo inyectado. Enter...")

        elif opcion == "4":

            reactor.temperatura = 25.0
            reactor.presion = 1.0
            reactor.bomba = 0.0
            reactor.valvula = 0

            reactor.revisar_interlocks()

        elif opcion == "5":

            reactor.actualizar_proceso()

        elif opcion == "6":

            return

        else:

            input("Opcion no valida. Enter...")


def menu_principal():
    reactor = Reactor()

    while True:

        reactor.revisar_interlocks()

        mostrar_hmi(reactor)

        print("\nMENU PRINCIPAL")

        print("1. Modo Manual")
        print("2. Modo Automatico")
        print("3. Modo de Pruebas")
        print("4. Salir")

        opcion = input("\nSeleccione una opcion: ").strip()

        if opcion == "1":

            modo_manual(reactor)

        elif opcion == "2":

            modo_automatico(reactor)

        elif opcion == "3":

            modo_pruebas(reactor)

        elif opcion == "4":

            limpiar_pantalla()

            print("Simulacion finalizada.")

            break

        else:

            input("Opcion invalida. Enter...")


if __name__ == "__main__":
    menu_principal()
    