#include "../include/Player.hpp"
#include "../include/GameBoard.hpp"

// Inicializa el tablero con los parámetros dados (usado en multijugador)
void Jugador::inicializarTablero(int filas, int columnas, int minas) {
    tablero = TableroJuego(TableroJuego::PERSONALIZADO, filas, columnas, minas);
    tablero.inicializar();
    vivo = true;
}

// Inicializa el tablero con una matriz de minas (sin aleatoriedad)
void Jugador::inicializarTableroConMinas(int filas, int columnas, const std::vector<std::vector<bool>>& minas) {
    tablero = TableroJuego(TableroJuego::PERSONALIZADO, filas, columnas, 0);
    tablero.setTableroMinas(minas);
    tablero.inicializar();
    vivo = true;
}

Jugador::Jugador(const std::string& nombre) : nombre(nombre), vivo(true) , tablero(TableroJuego::PRINCIPIANTE)/* inicializar con dificultad por defecto */{}

void Jugador::iniciarJuego(TableroJuego::Dificultad diff, int filasPersonalizadas, int columnasPersonalizadas, int minasPersonalizadas){
    tablero = TableroJuego(diff, filasPersonalizadas, columnasPersonalizadas, minasPersonalizadas);
    tablero.inicializar();
    vivo = true;
}

bool Jugador::realizarMovimiento(int fila, int columna){
    if (!vivo) return false;

    bool resultado = tablero.destapar(fila, columna);

    if(tablero.esJuegoTerminado()){
        vivo = false;
    }
    return resultado;
}

void Jugador::alternarBanderas(int fila, int columna){
    if (vivo){
        tablero.alternarBandera(fila, columna);
    }
}

TableroJuego& Jugador::obtenerTablero() {
    return tablero;
}

const TableroJuego& Jugador::obtenerTablero() const{
    return tablero;
}

bool Jugador::estaVivo() const{
    return vivo;
}

bool Jugador::haGanado() const {
    return tablero.esJuegoGanado();
}

int Jugador::obtenerPuntuacion() const {
    if(haGanado()){
        double tiempo = tablero.obtenerTiempoTranscurrido();
        //Fórmula de puntuación minas * 1000 / tiempo
        return static_cast<int>((tablero.obtenerMinas() * 1000) / (tiempo + 1));
    }
    return 0;
}

double Jugador::obtenerTiempo() const{
    return tablero.obtenerTiempoTranscurrido();
}

std::string Jugador::obtenerNombre() const {
    return nombre;
}