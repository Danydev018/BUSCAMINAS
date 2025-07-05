#include "../include/Player.hpp"

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