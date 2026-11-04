
from dispositivos import NodoSensor, NodoActuador


# Sistema de control de acceso
class SistemaAcceso:

    def __init__(self, lector, cerradura, tarjetas_validas):
        self.lector = lector
        self.cerradura = cerradura
        self.tarjetas_validas = tarjetas_validas

    def procesar_acceso(self, uid):

        self.lector.leer_tarjeta(uid)

        if self.lector.lectura_actual in self.tarjetas_validas:
            print("Tarjeta autorizada.")
            self.cerradura.encender()
            return True

        else:
            print("Tarjeta NO autorizada.")
            self.cerradura.apagar()
            return False


# Crear dispositivos
lector_rfid = NodoSensor(
    "RFID-001",
    "192.168.1.10",
    "Entrada principal"
)

cerradura = NodoActuador(
    "LOCK-001",
    "192.168.1.11",
    "Puerta principal"
)


# Configurar dispositivos
lector_rfid.establecer_limite(10)

lector_rfid.conectar()
cerradura.conectar()


# Tarjetas autorizadas
tarjetas_autorizadas = {
    "A1B2C3D4",
    "1234ABCD",
    "F1E2D3C4"
}


# Crear sistema
sistema = SistemaAcceso(
    lector_rfid,
    cerradura,
    tarjetas_autorizadas
)


# Lista de dispositivos IoT
red_iot = [
    lector_rfid,
    cerradura
]


# Menu principal
def main():

    print("\n===== CONTROL DE ACCESO RFID =====")

    while True:

        print("\n1. Leer tarjeta RFID")
        print("2. Mostrar telemetria")
        print("3. Salir")

        opcion = input("Selecciona una opcion: ")

        if opcion == "1":

            uid = input("Introduce el UID de la tarjeta: ")
            
            if uid.strip():
                sistema.procesar_acceso(uid)
            else:
                print("No se recibio ningun UID.")

        elif opcion == "2":

            print("\n===== MONITOREO DE RED IoT =====")

            # Bucle polimorfico
            for dispositivo in red_iot:
                dispositivo.enviar_telemetria()

        elif opcion == "3":

            cerradura.apagar()
            print("Sistema finalizado.")
            break

        else:
            print("Opcion no valida.")


if __name__ == "__main__":
    main()
