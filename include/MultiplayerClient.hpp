#ifndef MULTIPLAYERCLIENT_HPP
#define MULTIPLAYERCLIENT_HPP

#include <netinet/in.h>
#include "Player.hpp"

class ClienteMultijugador {
    private:
    std::string ip;
    int puerto, socketCliente;
    Jugador jugador;

    public:
    ClienteMultijugador(const std::string& ip, int puerto);
    bool conectar();
    void jugar();

};

#endif