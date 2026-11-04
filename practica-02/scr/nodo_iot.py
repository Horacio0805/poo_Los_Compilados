
class NodoIoT:

    # Constructor para guardar los datos del dispositivo
    def __init__(self, id_dispositivo, ip, ubicacion):
        self.id_dispositivo = id_dispositivo
        self.ip = ip
        self.ubicacion = ubicacion
        self.conectado = False
        self.bateria = 100
        self.datos_enviados = 0

    # Funcion para conectar el dispositivo
    def conectar(self):
        self.conectado = True
        print(f"{self.id_dispositivo}: conectado correctamente.")

    def desconectar(self):
        self.conectado = False
        print(f"{self.id_dispositivo}: se ha desconectado.")

    # Funcion para enviar los datos del nodo
    def enviar_telemetria(self):
        if self.conectado:
            print("\n--- Datos del dispositivo ---")
            print(f"ID: {self.id_dispositivo}")
            print(f"IP: {self.ip}")
            print(f"Ubicacion: {self.ubicacion}")
            print(f"Estado: Conectado")

            self.datos_enviados += 1
            self.bateria -= 1

            print(f"Bateria: {self.bateria}%")
            print(f"Datos enviados: {self.datos_enviados}")
        else:
            print("El dispositivo no esta conectado.")

    # Funcion para consultar el estado del dispositivo
    def mostrar_estado(self):
        print("\n--- Estado del nodo ---")
        print(f"Dispositivo: {self.id_dispositivo}")
        print(f"Direccion IP: {self.ip}")
        print(f"Ubicacion: {self.ubicacion}")
        print(f"Conexion: {self.conectado}")
        print(f"Bateria: {self.bateria}%")
        print(f"Total de datos enviados: {self.datos_enviados}")


nodo1 = NodoIoT("ESP32_01", "192.168.1.10", "Laboratorio")

nodo1.conectar()
nodo1.enviar_telemetria()
nodo1.enviar_telemetria()
nodo1.mostrar_estado()
nodo1.desconectar()
