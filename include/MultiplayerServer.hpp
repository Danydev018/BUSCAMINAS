#ifndef MULTIPLAYERSERVER_HPP
#define MULTIPLAYERSERVER_HPP

#include <netinet/in.h>
#include "Player.hpp"

class ServidorMultijugador {
    private:
    int socketServidor, socketCliente;
    sockaddr_in direccionServidor;
    sockaddr_in direccionCliente;
    Jugador jugadorAnfitrion;
    Jugador jugadorCliente;

    public:
    ServidorMultijugador(int puerto, TableroJuego::Dificultad diff, int filasPersonalizadas, int columnasPersonalizadas, int minasPersonalizadas);
    bool iniciar();
    void ejecutarJuego();
};

#endif