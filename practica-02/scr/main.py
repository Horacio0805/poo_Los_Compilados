from metodos_acceso import AccesoPIN, AccesoRFID, AccesoHuella


def mostrar_menu():
    print("\n===== CONTROL DE ACCESO AUTOMATICO =====")
    print("1. Acceso mediante PIN")
    print("2. Acceso mediante tarjeta RFID")
    print("3. Acceso mediante huella digital")
    print("4. Salir")


def crear_acceso(opcion, usuario):

    if opcion == "1":
        pin = input("Ingrese su PIN: ")
        return AccesoPIN(usuario, pin)

    elif opcion == "2":
        tarjeta = input("Ingrese el codigo RFID: ")
        return AccesoRFID(usuario, tarjeta)

    elif opcion == "3":
        huella = input("Ingrese el codigo de huella: ")
        return AccesoHuella(usuario, huella)

    return None


def main():

    while True:
        mostrar_menu()

        opcion = input("\nSeleccione una opcion: ").strip()

        if opcion == "4":
            print("\nSistema finalizado.")
            break

        if opcion not in ("1", "2", "3"):
            print("Error: opcion no valida.")
            continue

        usuario = input("Ingrese su nombre: ").strip()

        if not usuario:
            print("Error: debe ingresar un nombre.")
            continue

        try:
            acceso = crear_acceso(opcion, usuario)

            print("\nVerificando credenciales...")

            acceso.verificar_acceso()
            acceso.mostrar_estado()
            acceso.abrir_puerta()

        except ValueError as error:
            print("Error de validacion:", error)


if __name__ == "__main__":
    main()
