#include "../include/MultiplayerClient.hpp"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <thread>
#include "Utils.hpp"

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

void recibirActualizaciones(int socket,Jugador& jugador){
    char buffer[1024];
    while(true){
        ssize_t bytesRecibidos = recv(socket, buffer, sizeof(buffer)-1, 0);
        if(bytesRecibidos <= 0) break;

        buffer[bytesRecibidos] = '\0';

        std::string mensaje(buffer);

        if(mensaje.find("CONFIG") == 0){
            int filas, columnas, minas;
            
            sscanf(buffer, "CONFIG %d %d %d", &filas, &columnas, &minas);

            jugador.iniciarJuego(TableroJuego::PERSONALIZADO, filas, columnas, minas);
        }
        else if(mensaje.find("FIN") == 0){
            std::cout << "\n" << mensaje.substr(4) << "\n";
            break;
        }
        else if(mensaje.find("MOVE ") == 0){
            int fila, columna;
            char tipo;
            sscanf(buffer, "MOVE %d %d %c", &fila, &columna, &tipo);
            
            if(tipo == 'D') {
                jugador.realizarMovimiento(fila, columna);
            }else if(tipo == 'F'){
                jugador.alternarBanderas(fila, columna);
            }
        }
        else{
            //Actualización comleta del tablero
            TableroJuego& tablero = jugador.obtenerTablero();
            int filas = tablero.obtenerFilas();
            int columnas = tablero.obtenerColumnas();
            int index = 0;

            for(int f = 0; f < tablero.obtenerFilas(); f++){
                for(int c = 0; c < tablero.obtenerColumnas(); c++){
                   if(index < mensaje.size()){
                       tablero.obtenerTableroVisible()[f][c] = mensaje[index++];
                   }
                }
                if(index < mensaje.size() && mensaje[index] == '\n') index++;
            }
            tablero.imprimirTablero();
        }
    }
}

void ClienteMultijugador::jugar(){
    // Hilo para recibir actualizaciones
    std::thread hiloRecibir(recibirActualizaciones, socketCliente, std::ref(jugador));

    // Control de turnos: el cliente solo puede jugar cuando no es el turno del anfitrión
    bool miTurno = false;
    while (jugador.estaVivo() && !jugador.haGanado()) {
        // Esperar a que sea el turno del cliente (el servidor controla los turnos)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // El cliente puede jugar cuando el tablero se actualiza (por mensaje del servidor)
        // Para simplificar, permitimos que el cliente juegue siempre que esté vivo y no haya ganado
        jugador.obtenerTablero().imprimirTablero();

        int fila = obtenerEntrada("Fila: ", 0, jugador.obtenerTablero().obtenerFilas() - 1);
        int columna = obtenerEntrada("Columna: ", 0, jugador.obtenerTablero().obtenerColumnas() - 1);

        char accion;
        std::cout << "[D]estapar o [B]andera? ";
        std::cin >> accion;

        std::string movimientoMsg;
        if (accion == 'B' || accion == 'b') {
            jugador.alternarBanderas(fila, columna);
            movimientoMsg = "MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " F\n";
        } else {
            jugador.realizarMovimiento(fila, columna);
            movimientoMsg = "MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " D\n";
        }

        // Enviar movimiento al servidor
        send(socketCliente, movimientoMsg.c_str(), movimientoMsg.size(), 0);
    }

    hiloRecibir.join();
    close(socketCliente);
}