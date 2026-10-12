from metodos_acceso import AccesoPIN, AccesoRFID, AccesoHuella

while True:
    print("\n===== CONTROL DE ACCESO AUTOMATICO =====")
    print("1. Acceso mediante PIN")
    print("2. Acceso mediante tarjeta RFID")
    print("3. Acceso mediante huella digital")
    print("4. Salir")

    opcion = input("Seleccione una opcion: ")

    if opcion == "4":
        print("Sistema finalizado.")
        break

    if opcion not in ["1", "2", "3"]:
        print("Opcion no valida.")
        continue

    usuario = input("Ingrese su nombre: ")

    if opcion == "1":
        pin = input("Ingrese su PIN: ")
        acceso = AccesoPIN(usuario, pin)

    elif opcion == "2":
        tarjeta = input("Ingrese el codigo RFID: ")
        acceso = AccesoRFID(usuario, tarjeta)

    elif opcion == "3":
        huella = input("Ingrese el codigo de huella: ")
        acceso = AccesoHuella(usuario, huella)

    print("\nVerificando credenciales...")

    acceso.verificar_acceso()
    acceso.mostrar_estado()
    acceso.abrir_puerta()
