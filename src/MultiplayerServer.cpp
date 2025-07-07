#include "../include/MultiplayerServer.hpp"
#include "../include/Utils.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>
#include <sstream>
#include <iostream>

ServidorMultijugador::ServidorMultijugador(int puerto, TableroJuego::Dificultad diff, int filasPersonalizadas, int columnasPersonalizadas, int minasPersonalizadas) : jugadorAnfitrion("Anfitrion"), jugadorCliente("Cliente"){

    //Configurar dirección del servidor
    direccionServidor.sin_family = AF_INET;

    direccionServidor.sin_addr.s_addr = INADDR_ANY;
    direccionServidor.sin_port = htons(puerto);

    //Crear socket
    socketServidor = socket(AF_INET, SOCK_STREAM, 0);
    if(socketServidor == -1){
        std::cerr << "Error al crear el socket del servidor\n";
        return;
    }

    //Permitir reutilizar puerto
    int opt = 1;
    setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //Configurar juego
    jugadorAnfitrion.iniciarJuego(diff, filasPersonalizadas, columnasPersonalizadas, minasPersonalizadas);
}

bool ServidorMultijugador::iniciar(){
    //Enlazar socket
    if(bind(socketServidor, (struct sockaddr*)&direccionServidor, sizeof(direccionServidor))){
        std::cerr << "Error en bind\n";
        return false;
    }

    //Escuchar
    if(listen(socketServidor, 1)){
        std::cerr << "Error en listen\n";
        return false;
    }

    std::cout << "Esperando conexión del cliente...\n";
    socklen_t tamanoCliente = sizeof(direccionCliente);
    socketCliente = accept(socketServidor, (struct sockaddr*)&direccionCliente, &tamanoCliente);
    if(socketCliente < 0){
        std::cerr << "Error en accept\n";
        return false;
    }

    std::cout << "Cliente conectado\n";
    return true;
}

void enviarTablero(int socket, const TableroJuego& tablero){
    std::stringstream ss;
    for(int f = 0; f < tablero.obtenerFilas(); f++){
        for(int c = 0; c < tablero.obtenerColumnas(); c++){
            ss << tablero.obtenerTableroVisible()[f][c];
        }
        ss <<"\n";
    }
    std::string tableroStr = ss.str();
    send(socket, tableroStr.c_str(), tableroStr.size(), 0);
}

void ServidorMultijugador::ejecutarJuego(){
    //Enviar configuración inicial al cliente
    TableroJuego& tablero = jugadorAnfitrion.obtenerTablero();
    std::string configMsg = "CONFIG " + std::to_string(tablero.obtenerFilas()) + " " + std::to_string(tablero.obtenerColumnas()) + " " + std::to_string(tablero.obtenerMinas()) + "\n";
    send(socketCliente, configMsg.c_str(), configMsg.size(), 0);
    // Enviar la matriz de minas al cliente (serializada)
    const auto& minas = tablero.getTableroMinas();
    std::string minasMsg = "MINAS ";
    for (int f = 0; f < tablero.obtenerFilas(); ++f) {
        for (int c = 0; c < tablero.obtenerColumnas(); ++c) {
            minasMsg += minas[f][c] ? '1' : '0';
        }
    }
    minasMsg += "\n";
    send(socketCliente, minasMsg.c_str(), minasMsg.size(), 0);

    // Control de turnos: true = anfitrión, false = cliente
    bool turnoAnfitrion = true;
    bool juegoTerminado = false;

    // Hilo para recibir movimientos del cliente
    std::thread hiloRecibir([&](){
        char buffer[1024];
        while (!juegoTerminado) {
            ssize_t bytesRecibidos = recv(socketCliente, buffer, sizeof(buffer) - 1, 0);
            if (bytesRecibidos <= 0) break;
            buffer[bytesRecibidos] = '\0';
            std::string mensaje(buffer);

            if (mensaje.find("MOVE ") == 0) {
                int fila, columna;
                char tipo;
                sscanf(buffer, "MOVE %d %d %c", &fila, &columna, &tipo);

                if (tipo == 'F' || tipo == 'f') {
                    jugadorCliente.alternarBanderas(fila, columna);
                    std::cout << "El rival ha colocado una bandera\n";
                    std::string notif = "NOTIF El rival ha colocado una bandera\n";
                    send(socketCliente, notif.c_str(), notif.size(), 0);
                    // NO alternar turno ni enviar TURN
                } else if (tipo == 'D' || tipo == 'd') {
                    // Validar que la celda NO tenga bandera antes de destapar
                    char visible = jugadorCliente.obtenerTablero().obtenerTableroVisible()[fila][columna];
                    if (visible == 'B') {
                        std::string notif = "NOTIF No puedes destapar una celda con bandera\n";
                        send(socketCliente, notif.c_str(), notif.size(), 0);
                        // NO alternar turno ni hacer nada
                        continue;
                    }
                    bool esBomba = jugadorCliente.obtenerTablero().getTableroMinas()[fila][columna];
                    jugadorCliente.realizarMovimiento(fila, columna);
                    jugadorAnfitrion.realizarMovimiento(fila, columna);
                    if (esBomba) {
                        std::string loseMsg = "LOSE Has perdido. Has destapado una bomba.\n";
                        send(socketCliente, loseMsg.c_str(), loseMsg.size(), 0);
                        std::cout << "¡Has ganado! El rival destapó una bomba.\n";
                        juegoTerminado = true;
                    } else {
                        // Ya no se envía el tablero, solo alternar turno y enviar TURN
                        if (!juegoTerminado) {
                            turnoAnfitrion = true;
                            std::string turnMsg = "TURN\n";
                            send(socketCliente, turnMsg.c_str(), turnMsg.size(), 0);
                        }
                    }
                }
            }
        }
    });

    // Juego principal por turnos
    // --- NUEVO: Entrada conjunta y recuadro ---
    #include "../include/UtilsExtra.hpp"
    int baseX = 8, baseY = 2;
    int cellW = 5;
    int ancho = tablero.obtenerColumnas() * cellW + 1;
    while (!juegoTerminado && jugadorAnfitrion.estaVivo() && jugadorCliente.estaVivo() && !jugadorAnfitrion.haGanado() && !jugadorCliente.haGanado()) {
        int filaEntrada = baseY + 6 + tablero.obtenerFilas() * 2;
        if (turnoAnfitrion) {
            jugadorAnfitrion.obtenerTablero().imprimirTablero();
            std::string entrada;
            int fila = -1, columna = -1;
            char accion = ' ';
            bool entradaValida = false;
            do {
                limpiarZonaEntrada(baseX, filaEntrada, ancho);
                imprimirRecuadroEntrada(baseX, filaEntrada, ancho, "Ingrese fila columna accion (D/B): ");
                std::getline(std::cin >> std::ws, entrada);
                std::istringstream iss(entrada);
                if (!(iss >> fila >> columna >> accion)) {
                    limpiarZonaEntrada(baseX, filaEntrada, ancho);
                    imprimirRecuadroEntrada(baseX, filaEntrada, ancho, "Entrada inválida. Intente: fila columna D/B");
                    continue;
                }
                if (fila < 0 || fila >= tablero.obtenerFilas() || columna < 0 || columna >= tablero.obtenerColumnas() || !(accion == 'D' || accion == 'd' || accion == 'B' || accion == 'b')) {
                    limpiarZonaEntrada(baseX, filaEntrada, ancho);
                    imprimirRecuadroEntrada(baseX, filaEntrada, ancho, "Entrada inválida. Intente: fila columna D/B");
                    continue;
                }
                entradaValida = true;
            } while (!entradaValida);
            limpiarZonaEntrada(baseX, filaEntrada, ancho);
            if (accion == 'B' || accion == 'b') {
                jugadorAnfitrion.alternarBanderas(fila, columna);
                jugadorAnfitrion.obtenerTablero().imprimirTablero();
                std::string notif = "NOTIF El rival ha colocado una bandera\n";
                send(socketCliente, notif.c_str(), notif.size(), 0);
                // NO alternar turno ni enviar TURN
            } else {
                // Validar que la celda NO tenga bandera antes de destapar
                char visible = jugadorAnfitrion.obtenerTablero().obtenerTableroVisible()[fila][columna];
                if (visible == 'B') {
                    std::string notif = "NOTIF No puedes destapar una celda con bandera\n";
                    send(socketCliente, notif.c_str(), notif.size(), 0);
                    // NO alternar turno ni hacer nada
                    continue;
                }
                bool esBomba = tablero.getTableroMinas()[fila][columna];
                jugadorAnfitrion.realizarMovimiento(fila, columna);
                jugadorAnfitrion.obtenerTablero().imprimirTablero();
                jugadorCliente.realizarMovimiento(fila, columna);
                if (esBomba) {
                    std::string loseMsg = "LOSE Has perdido. Has destapado una bomba.\n";
                    send(socketCliente, loseMsg.c_str(), loseMsg.size(), 0);
                    std::cout << "¡Has perdido! Has destapado una bomba.\n";
                    juegoTerminado = true;
                } else {
                    // Ya no se envía el tablero, solo alternar turno y enviar TURN
                    if (!juegoTerminado) {
                        turnoAnfitrion = false;
                        limpiarZonaEntrada(baseX, filaEntrada, ancho);
                        imprimirRecuadroEntrada(baseX, filaEntrada, ancho, "Esperando el movimiento del rival.....");
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        std::string turnMsg = "TURN\n";
                        send(socketCliente, turnMsg.c_str(), turnMsg.size(), 0);
                    }
                }
            }
        } else {
            static bool esperandoTurno = false;
            if (!esperandoTurno) {
                jugadorAnfitrion.obtenerTablero().imprimirTablero();
                limpiarZonaEntrada(baseX, filaEntrada, ancho);
                imprimirRecuadroEntrada(baseX, filaEntrada, ancho, "Esperando el movimiento del rival.....");
                esperandoTurno = true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
    }

    // Finalizar Juego
    juegoTerminado = true;
    std::string resultado;
    if (jugadorAnfitrion.haGanado()) {
        resultado = "GANADOR Anfitrion";
    } else if (jugadorCliente.haGanado()) {
        resultado = "GANADOR Cliente";
    } else {
        resultado = "EMPATE";
    }

    // Mostrar mensaje de fin de juego localmente (usar variables ya declaradas)
    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
    imprimirRecuadroEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho, resultado);
    std::this_thread::sleep_for(std::chrono::seconds(4));
    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);

    std::string finMsg = "FIN " + resultado + "\n";
    send(socketCliente, finMsg.c_str(), finMsg.size(), 0);

    hiloRecibir.join();
    close(socketCliente);
    close(socketServidor);
}