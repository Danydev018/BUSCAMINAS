#ifndef PLAYER_HPP
 #define PLAYER_HPP

#include <string>
#include "GameBoard.hpp"

class Jugador{
    private:
    std::string nombre;
    TableroJuego tablero;
    bool vivo;

    public:
    Jugador(const std::string& nombre);

    void iniciarJuego(TableroJuego::Dificultad diff, int filasPersonalizadas = 8, int columnasPersonalizadas = 8, int minasPersonalizadas = 10);
    void alternarBanderas(int fila, int columna);

    bool realizarMovimiento(int fila, int columna);
    bool estaVivo() const;
    bool haGanado() const;

    int obtenerPuntuacion() const;
    double obtenerTiempo() const;
    std::string obtenerNombre() const;
    TableroJuego& obtenerTablero();
    const TableroJuego& obtenerTablero() const;
    
    // Inicializa el tablero con los parámetros dados (usado en multijugador)
    void inicializarTablero(int filas, int columnas, int minas);
    // Inicializa el tablero con una matriz de minas (sin aleatoriedad)
    void inicializarTableroConMinas(int filas, int columnas, const std::vector<std::vector<bool>>& minas);
};

#endif