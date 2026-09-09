import os


# ---------------------------------------------------------------------------
# Límites y constantes de operación
# ---------------------------------------------------------------------------
TEMP_MIN, TEMP_MAX = 0.0, 150.0
PRES_MIN, PRES_MAX = 0.0, 15.0

TEMP_INTERLOCK = 85.0      # °C
PRES_INTERLOCK = 12.0      # Bar

CAUDAL_MAX = 25.0          # L/min, caudal a bomba 100% (para "leer caudal")

MODO_MANUAL, MODO_AUTOMATICO, MODO_PRUEBAS = "MANUAL", "AUTOMATICO", "PRUEBAS"


# ---------------------------------------------------------------------------
# Estado del reactor (planta simulada)
# ---------------------------------------------------------------------------
class Reactor:
    def __init__(self):
        self.temperatura = 25.0        # °C
        self.presion = 1.0             # Bar
        self.bomba_pct = 0.0           # % (0-100)
        self.valvula_abierta = False   # False = 0, True = 1
        self.modo = MODO_MANUAL
        self.alarma_activa = False
        self.ultimo_mensaje = ""

    # -- Actuadores ---------------------------------------------------------
    def set_bomba(self, valor):
        self.bomba_pct = max(0.0, min(100.0, valor))

    def set_valvula(self, abierta: bool):
        self.valvula_abierta = abierta

    # -- Lógica de modo automático -------------------------------------------
    def paso_automatico(self):
        """ΔT = (+1.5°C) - (0.05°C x %Bomba) por cada ciclo de control."""
        delta_t = 1.5 - 0.05 * self.bomba_pct
        self.temperatura += delta_t
        self.temperatura = max(TEMP_MIN, min(TEMP_MAX, self.temperatura))
        # La presión reacciona de forma acoplada a la temperatura
        # (relación simplificada para efectos de simulación).
        self.presion = max(PRES_MIN, min(PRES_MAX, 1.0 + 0.09 * (self.temperatura - 25.0)))

    # -- Interlock de seguridad (prioridad máxima, se evalúa siempre) -------
    def verificar_interlock(self):
        if self.temperatura > TEMP_INTERLOCK or self.presion > PRES_INTERLOCK:
            self.set_bomba(100.0)
            self.set_valvula(True)
            self.alarma_activa = True
            return True
        self.alarma_activa = False
        return False

    # -- Lecturas de instrumentos --------------------------------------------
    def leer_caudal(self):
        """Caudal de la bomba de enfriamiento, proporcional al % de operación."""
        return (self.bomba_pct / 100.0) * CAUDAL_MAX

    def leer_manometro(self):
        return self.presion


reactor = Reactor()


# ---------------------------------------------------------------------------
# HMI de consola
# ---------------------------------------------------------------------------
def limpiar_pantalla():
    os.system('cls' if os.name == 'nt' else 'clear')


def dibujar_panel():
    limpiar_pantalla()
    print("=" * 58)
    print("   SIMULADOR DE REACTOR QUIMICO - Panel HMI (v1.0.0)")
    print("=" * 58)
    print(f" Modo actual        : {reactor.modo}")
    estado_alarma = "*** ALARMA - INTERLOCK ACTIVO ***" if reactor.alarma_activa else "Normal"
    print(f" Estado de seguridad: {estado_alarma}")
    print("-" * 58)
    print(f" Temperatura        : {reactor.temperatura:6.2f} °C   (limite {TEMP_INTERLOCK} °C)")
    print(f" Presion            : {reactor.presion:6.2f} Bar  (limite {PRES_INTERLOCK} Bar)")
    print(f" Bomba de Enfriam.  : {reactor.bomba_pct:6.2f} %")
    print(f" Valvula de Alivio  : {'ABIERTA' if reactor.valvula_abierta else 'CERRADA'}")
    print("-" * 58)
    if reactor.ultimo_mensaje:
        print(f" > {reactor.ultimo_mensaje}")
        print("-" * 58)
    print(" Comandos: leer caudal | leer manometro | estado")
    if reactor.modo == MODO_MANUAL:
        print("           bomba <0-100> | valvula on/off")
    elif reactor.modo == MODO_PRUEBAS:
        print("           forzar temperatura <valor> | forzar presion <valor>")
    print("           modo manual | modo automatico | modo pruebas | salir")
    print("=" * 58)


def procesar_comando(entrada):
    partes = entrada.strip().lower().split()
    if not partes:
        reactor.ultimo_mensaje = ""
        return True

    cmd = partes[0]

    # -- Comandos disponibles en cualquier modo --------------------------
    if cmd == "leer" and len(partes) >= 2 and partes[1] == "caudal":
        reactor.ultimo_mensaje = f"Caudal de bomba: {reactor.leer_caudal():.2f} L/min"
        return True

    if cmd == "leer" and len(partes) >= 2 and partes[1] == "manometro":
        reactor.ultimo_mensaje = f"Manometro (presion): {reactor.leer_manometro():.2f} Bar"
        return True

    if cmd == "estado":
        reactor.ultimo_mensaje = "Panel actualizado."
        return True

    if cmd == "modo" and len(partes) >= 2:
        if partes[1] == "manual":
            reactor.modo = MODO_MANUAL
            reactor.ultimo_mensaje = "Modo cambiado a MANUAL."
        elif partes[1] in ("automatico", "automático"):
            reactor.modo = MODO_AUTOMATICO
            reactor.ultimo_mensaje = "Modo cambiado a AUTOMATICO."
        elif partes[1] == "pruebas":
            reactor.modo = MODO_PRUEBAS
            reactor.ultimo_mensaje = "Modo cambiado a PRUEBAS."
        else:
            reactor.ultimo_mensaje = "Modo no reconocido."
        return True

    if cmd == "salir":
        return False

    # -- Comandos exclusivos de Modo Manual ------------------------------
    if reactor.modo == MODO_MANUAL:
        if cmd == "bomba" and len(partes) >= 2:
            if reactor.alarma_activa:
                reactor.ultimo_mensaje = "Instruccion ignorada: interlock de seguridad activo."
                return True
            try:
                reactor.set_bomba(float(partes[1]))
                reactor.ultimo_mensaje = f"Bomba ajustada a {reactor.bomba_pct:.2f} %."
            except ValueError:
                reactor.ultimo_mensaje = "Valor invalido para bomba."
            return True

        if cmd == "valvula" and len(partes) >= 2:
            if reactor.alarma_activa:
                reactor.ultimo_mensaje = "Instruccion ignorada: interlock de seguridad activo."
                return True
            if partes[1] == "on":
                reactor.set_valvula(True)
                reactor.ultimo_mensaje = "Valvula ABIERTA."
            elif partes[1] == "off":
                reactor.set_valvula(False)
                reactor.ultimo_mensaje = "Valvula CERRADA."
            else:
                reactor.ultimo_mensaje = "Use: valvula on|off"
            return True

    # -- Comandos exclusivos de Modo Pruebas (inyeccion de fallos) -------
    if reactor.modo == MODO_PRUEBAS:
        if cmd == "forzar" and len(partes) >= 3 and partes[1] == "temperatura":
            try:
                reactor.temperatura = max(TEMP_MIN, min(TEMP_MAX, float(partes[2])))
                reactor.ultimo_mensaje = f"Falla inyectada: temperatura forzada a {reactor.temperatura:.2f} °C."
            except ValueError:
                reactor.ultimo_mensaje = "Valor invalido."
            return True

        if cmd == "forzar" and len(partes) >= 3 and partes[1] == "presion":
            try:
                reactor.presion = max(PRES_MIN, min(PRES_MAX, float(partes[2])))
                reactor.ultimo_mensaje = f"Falla inyectada: presion forzada a {reactor.presion:.2f} Bar."
            except ValueError:
                reactor.ultimo_mensaje = "Valor invalido."
            return True

    reactor.ultimo_mensaje = "Comando no reconocido para el modo actual."
    return True


def main():
    continuar = True
    while continuar:
        # El interlock se evalua en CADA ciclo, sin importar el modo,
        # y tiene prioridad sobre cualquier instruccion del operario.
        reactor.verificar_interlock()

        if reactor.modo == MODO_AUTOMATICO and not reactor.alarma_activa:
            reactor.paso_automatico()
            reactor.verificar_interlock()

        dibujar_panel()
        try:
            entrada = input("Comando> ")
        except (EOFError, KeyboardInterrupt):
            break

        continuar = procesar_comando(entrada)

    print("\nSimulador finalizado.")


if __name__ == "__main__":
    main()
