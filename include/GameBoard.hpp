#ifndef GAMEBOARD_HPP
#define GAMEBOARD_HPP

#include <vector>
#include <ctime>
#include <string>

class TableroJuego {
public:
    enum Dificultad {
        PRINCIPIANTE,
        INTERMEDIO,
        EXPERTO,
        PERSONALIZADO
    };
private:
    void colocarMinas();
    void calcularMinasAdyacentes();
    void revelarCeldasAdyacentes(int fila, int columna);
    bool esPosicionValida(int fila, int columna) const;

    int filas, columnas, minas, celdasDestapadas, banderasColocadas;
    bool juegoTerminado, juegoGanado;
    Dificultad dificultad;

    std::vector<std::vector<char>>tableroVisible;
    std::vector<std::vector<bool>>tableroMinas;
    std::vector<std::vector<int>>tableroAdyacentes;

    time_t tiempoInicio;
    time_t tiempoFin;

public:
    TableroJuego(Dificultad diff, int filasPersonalizadas = 8, int columnasPersonalizadas = 8, int minasPersonalizadas = 10);
    void inicializar();
    bool destapar(int fila, int columna);
    void alternarBandera(int fila, int columna);
    void imprimirTablero() const;
    void iniciarCronometro();
    void detenerCronometro();

    int obtenerCeldasDestapadas() const;
    int obtenerFilas() const;
    int obtenerColumnas() const;
    int obtenerMinas() const;
    bool esJuegoTerminado() const;
    bool esJuegoGanado() const;
    double obtenerTiempoTranscurrido() const;

    std::vector<std::vector<char>>& obtenerTableroVisible() { return tableroVisible; }
    const std::vector<std::vector<char>>& obtenerTableroVisible() const { return tableroVisible; }

    // Permite inicializar el tablero de minas desde una matriz externa (para multijugador)
    void setTableroMinas(const std::vector<std::vector<bool>>& minasExternas);
    // Permite obtener la matriz de minas (para enviar al cliente)
    const std::vector<std::vector<bool>>& getTableroMinas() const;
};

#endif