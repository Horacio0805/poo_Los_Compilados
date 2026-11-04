
from nodo_iot import NodoIoT
from numbers import Real


# Clase hija: lector RFID
class NodoSensor(NodoIoT):

    def __init__(self, id_dispositivo, ip, ubicacion):
        super().__init__(id_dispositivo, ip, ubicacion)

        self.__limite_alerta = 0.0
        self.lectura_actual = "Sin tarjeta"

    def establecer_limite(self, limite):

        if isinstance(limite, bool) or not isinstance(limite, Real):
            raise ValueError("El limite debe ser numerico.")

        if limite < 0:
            raise ValueError("El limite no puede ser negativo.")

        self.__limite_alerta = float(limite)

    def leer_tarjeta(self, uid):
        self.lectura_actual = uid.strip().upper()
        print(f"Tarjeta detectada: {self.lectura_actual}")

    def enviar_telemetria(self):

        print("\n--- TELEMETRIA DEL LECTOR RFID ---")
        print(f"ID: {self.id_dispositivo}")
        print(f"IP: {self.ip}")
        print(f"Ubicacion: {self.ubicacion}")
        print(f"UID detectado: {self.lectura_actual}")
        print(f"Limite: {self.__limite_alerta}")
        print(f"Conectado: {self.conectado}")


# Clase hija: cerradura electronica
class NodoActuador(NodoIoT):

    def __init__(self, id_dispositivo, ip, ubicacion):
        super().__init__(id_dispositivo, ip, ubicacion)

        self.__estado_activo = False
        self.__potencia_pwm = 0

    def encender(self):
        self.__estado_activo = True
        self.__potencia_pwm = 100
        print("ACCESO CONCEDIDO: Cerradura desbloqueada.")

    def apagar(self):
        self.__estado_activo = False
        self.__potencia_pwm = 0
        print("Cerradura bloqueada.")

    def enviar_telemetria(self):

        estado = "Desbloqueada" if self.__estado_activo else "Bloqueada"

        print("\n--- TELEMETRIA DE LA CERRADURA ---")
        print(f"ID: {self.id_dispositivo}")
        print(f"IP: {self.ip}")
        print(f"Ubicacion: {self.ubicacion}")
        print(f"Estado: {estado}")
        print(f"Potencia PWM: {self.__potencia_pwm}%")
        print(f"Conectado: {self.conectado}")
