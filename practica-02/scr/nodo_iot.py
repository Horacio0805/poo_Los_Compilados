
class NodoIoT:

    def __init__(self, id_dispositivo, ip, ubicacion):
        self.id_dispositivo = id_dispositivo
        self.ip = ip
        self.ubicacion = ubicacion
        self.conectado = False

    def conectar(self):
        self.conectado = True
        print(f"{self.id_dispositivo}: conectado correctamente.")

    def enviar_telemetria(self):
        print(f"Dispositivo: {self.id_dispositivo}")
        print(f"IP: {self.ip}")
        print(f"Ubicacion: {self.ubicacion}")
        print(f"Conexion: {self.conectado}")
