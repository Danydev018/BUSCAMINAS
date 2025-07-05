#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include "GameBoard.hpp"

std::string obtenerFechaHoraActual();
void limpiarPantalla();
// Mueve el cursor a la posición (x, y) en la terminal (1-indexed)
void gotoxy(int x, int y);
int obtenerEntrada(const std::string& mensaje, int min, int max);
std::string dificultadAString(TableroJuego::Dificultad diff);

#endif

// Imprime una línea alineada dentro de un marco, compensando ancho visual (para tablero)
void printBoardLine(int baseX, int y, const std::string& content, int ancho);