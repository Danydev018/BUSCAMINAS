#include "../include/MultiplayerClient.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <atomic>
#include "Utils.hpp"
#include "UtilsExtra.hpp"
#include <sstream>

void recibirActualizaciones(int socket, Jugador& jugador);

ClienteMultijugador::ClienteMultijugador(const std::string& ip, int puerto) : ip(ip), puerto(puerto), jugador("Cliente"){}

bool ClienteMultijugador::conectar() {
    socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if(socketCliente == -1){
        std::cerr << "Error al crear socket del cliente\n";
        return false;
    }

    sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(puerto);
    inet_pton(AF_INET, ip.c_str(), &direccionServidor.sin_addr);

    if(connect(socketCliente,(struct sockaddr*)&direccionServidor, sizeof(direccionServidor))){
        std::cerr << "Error al conectar\n";
        return false;
    }

    std::cout << "Conectando al servidor\n";
    return true;
}

// Flag local para el control de turnos
static bool flag_turno = false; // El cliente inicia esperando el movimiento del servidor

// Función auxiliar para mostrar todas las minas y banderas negras
void mostrarBombasYBanderas(TableroJuego& tablero) {
    auto& minas = tablero.getTableroMinas();
    auto& visible = tablero.obtenerTableroVisible();
    int filas = tablero.obtenerFilas();
    int columnas = tablero.obtenerColumnas();
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            if (minas[f][c]) {
                // Si ya hay bandera, poner bandera negra, si no, mostrar bomba
                if (visible[f][c] == 'B') {
                    visible[f][c] = 'X'; // Bandera negra
                } else {
                    visible[f][c] = '*'; // Bomba
                }
            }
        }
    }
    tablero.imprimirTablero();
}

void recibirActualizaciones(int socket,Jugador& jugador){
    char buffer[2048];
    std::string bufferAcumulado;
    bool tablero_inicializado = false;
    int filas = 0, columnas = 0, minas = 0;
    std::vector<std::vector<bool>> matrizMinas;
    while(true){
        if (!flag_turno) {
            ssize_t bytesRecibidos = recv(socket, buffer, sizeof(buffer)-1, 0);
            if(bytesRecibidos <= 0) {
                std::cerr << "\nConexión perdida con el servidor.\n";
                exit(1);
            }
            buffer[bytesRecibidos] = '\0';
            bufferAcumulado += buffer;
            size_t pos;
            while ((pos = bufferAcumulado.find('\n')) != std::string::npos) {
                std::string mensaje = bufferAcumulado.substr(0, pos);
                bufferAcumulado.erase(0, pos + 1);

                // Procesar mensaje de configuración inicial
                if(mensaje.find("CONFIG ") == 0) {
                    sscanf(mensaje.c_str(), "CONFIG %d %d %d", &filas, &columnas, &minas);
                    continue;
                }
                // Procesar matriz de minas
                if(mensaje.find("MINAS ") == 0) {
                    std::string minasStr = mensaje.substr(6); // Quitar "MINAS "
                    if ((int)minasStr.size() < filas * columnas) {
                        // Esperar a recibir el mensaje completo
                        bufferAcumulado = mensaje + "\n" + bufferAcumulado;
                        break;
                    }
                    matrizMinas.clear();
                    int idx = 0;
                    for(int f = 0; f < filas; ++f) {
                        std::vector<bool> fila;
                        for(int c = 0; c < columnas; ++c) {
                            fila.push_back(minasStr[idx++] == '1');
                        }
                        matrizMinas.push_back(fila);
                    }
                    jugador.inicializarTableroConMinas(filas, columnas, matrizMinas);
                    tablero_inicializado = true;
                    jugador.obtenerTablero().imprimirTablero();
                    continue;
                }
                // No procesar nada si el tablero no está inicializado
                if (!tablero_inicializado) continue;

                if(mensaje.find("MOVE ") == 0){
                    int fila, columna;
                    char tipo;
                    sscanf(mensaje.c_str(), "MOVE %d %d %c", &fila, &columna, &tipo);
                    if(tipo == 'D' || tipo == 'd') {
                        jugador.realizarMovimiento(fila, columna);
                    } else if(tipo == 'F' || tipo == 'f') {
                        jugador.alternarBanderas(fila, columna);
                    }
                    jugador.obtenerTablero().imprimirTablero();
                    // Cambiar turno local
                    flag_turno = true;
                } else if(mensaje.find("NOTIF") == 0) {
                    TableroJuego& tablero = jugador.obtenerTablero();
                    tablero.imprimirTablero();
                    int baseX = 8, baseY = 2;
                    int cellW = 5;
                    int ancho = tablero.obtenerColumnas() * cellW + 1;
                    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
                    imprimirRecuadroEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho, mensaje.substr(6));
                } else if(mensaje.find("LOSE") == 0) {
                    TableroJuego& tablero = jugador.obtenerTablero();
                    mostrarBombasYBanderas(tablero);
                    int baseX = 8, baseY = 2;
                    int cellW = 5;
                    int ancho = tablero.obtenerColumnas() * cellW + 1;
                    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
                    imprimirRecuadroEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho, mensaje.substr(5));
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
                } else if(mensaje.find("FIN") == 0){
                    int baseX = 8, baseY = 2;
                    int cellW = 5;
                    TableroJuego& tablero = jugador.obtenerTablero();
                    int ancho = tablero.obtenerColumnas() * cellW + 1;
                    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
                    imprimirRecuadroEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho, mensaje.substr(4));
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
                    exit(0);
                } else if(mensaje.find("GANADOR") == 0 || mensaje.find("EMPATE") == 0) {
                    int baseX = 8, baseY = 2;
                    int cellW = 5;
                    TableroJuego& tablero = jugador.obtenerTablero();
                    int ancho = tablero.obtenerColumnas() * cellW + 1;
                    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
                    imprimirRecuadroEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho, mensaje);
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                    limpiarZonaEntrada(baseX, baseY+6+tablero.obtenerFilas()*2, ancho);
                    exit(0);
                }
                // Ignorar cualquier mensaje que no sea de control o movimiento
            }
        }
    }
}

void ClienteMultijugador::jugar(){
    // Hilo para recibir actualizaciones
    std::thread hiloRecibir(recibirActualizaciones, socketCliente, std::ref(jugador));

    int baseX = 8, baseY = 2;
    int cellW = 5;
    bool esperandoTurno = false;
    while (jugador.estaVivo() && !jugador.haGanado()) {
        TableroJuego& tablero = jugador.obtenerTablero();
        int ancho = tablero.obtenerColumnas() * cellW + 1;
        int filaEntrada = baseY + 6 + tablero.obtenerFilas() * 2;
        if (flag_turno) {
            tablero.imprimirTablero();
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
            std::string movimientoMsg;
            if (accion == 'B' || accion == 'b') {
                jugador.alternarBanderas(fila, columna);
                tablero.imprimirTablero();
                movimientoMsg = "MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " F\n";
                send(socketCliente, movimientoMsg.c_str(), movimientoMsg.size(), 0);
                // NO alternar turno
            } else {
                // Validar que la celda NO tenga bandera antes de enviar el movimiento
                if (tablero.obtenerTableroVisible()[fila][columna] == 'B') {
                    limpiarZonaEntrada(baseX, filaEntrada, ancho);
                    imprimirRecuadroEntrada(baseX, filaEntrada, ancho, "No puedes destapar una celda con bandera");
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                    continue;
                }
                jugador.realizarMovimiento(fila, columna);
                tablero.imprimirTablero();
                movimientoMsg = "MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " D\n";
                send(socketCliente, movimientoMsg.c_str(), movimientoMsg.size(), 0);
                // Cambiar turno local
                flag_turno = false;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    hiloRecibir.join();
    close(socketCliente);
}