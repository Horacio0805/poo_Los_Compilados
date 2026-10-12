
class ControlAcceso:
    def __init__(self, usuario):
        self.usuario = usuario
        self.acceso_permitido = False

    def verificar_acceso(self):
        raise NotImplementedError(
            "La subclase debe implementar este metodo"
        )

    def abrir_puerta(self):
        if self.acceso_permitido:
            print("Puerta abierta. Bienvenido,", self.usuario)
        else:
            print("Acceso denegado. Puerta bloqueada.")

    def mostrar_estado(self):
        estado = "Permitido" if self.acceso_permitido else "Denegado"
        print("Usuario:", self.usuario)
        print("Estado del acceso:", estado)
