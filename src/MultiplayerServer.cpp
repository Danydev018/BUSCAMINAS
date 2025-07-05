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

    //Hilo para recibir movimientos del ciente
    std::thread hiloRecibir([&](){
        char buffer[1024];
        while(true){
            ssize_t bytesRecibidos = recv(socketCliente, buffer, sizeof(buffer), 0);
            if(bytesRecibidos <= 0) break;
            buffer[bytesRecibidos] = '\0';
            std::string mensaje(buffer);

            if(mensaje.find("MOVE ") == 0){
                int fila, columna;
                char tipo;
                sscanf(buffer, "MOVE %d %d %c", &fila, &columna, &tipo);

                if(tipo == 'D'){
                    jugadorCliente.realizarMovimiento(fila, columna);
                }else if(tipo == 'F'){
                    jugadorCliente.alternarBanderas(fila, columna);
                }
                //Enviar actualización del tablero
                enviarTablero(socketCliente, jugadorAnfitrion.obtenerTablero());
            }
        }
    });

    //Juego principal
    while(jugadorAnfitrion.estaVivo() && jugadorCliente.estaVivo() && !jugadorAnfitrion.haGanado() && !jugadorCliente.haGanado()){

        //Turno del anfitrion
        jugadorAnfitrion.obtenerTablero().imprimirTablero();

        int fila = obtenerEntrada("Fila: ", 0, tablero.obtenerFilas()-1);
        int columna = obtenerEntrada("Columna: ", 0, tablero.obtenerColumnas()-1);

        char accion;
        std::cout << "[D]estapar o [B]andera? ";
        std::cin >> accion;

        std::string movimientoMsg;
        if(accion == 'B' || accion == 'b'){
            jugadorAnfitrion.alternarBanderas(fila, columna);

            movimientoMsg = "MOVE " + std::to_string(fila) + " " + std::to_string(columna) + " F\n";
        }else{
            jugadorAnfitrion.realizarMovimiento(fila, columna);
            movimientoMsg = "Move " + std::to_string(fila) + " " + std::to_string(columna) + " D\n";
        }

        //Enviar movimiento a cliente
        send(socketCliente, movimientoMsg.c_str(), movimientoMsg.size(), 0);
        enviarTablero(socketCliente, jugadorAnfitrion.obtenerTablero());
    }
    // Finalizar Juego
    std::string resultado;
    if(!jugadorAnfitrion.haGanado()){
        resultado = "GANADOR Anfitrion";
    }else if(jugadorCliente.haGanado()){
        resultado = "GANADOR Cliente";
    }else{
        resultado = "EMPATE";
    }
   
    std::string finMsg = "FIN " + resultado + "\n";
    send(socketCliente, finMsg.c_str(), finMsg.size(), 0);

    hiloRecibir.join();
    close(socketCliente);
    close(socketServidor);
}