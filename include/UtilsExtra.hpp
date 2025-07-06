#ifndef UTILS_EXTRA_HPP
#define UTILS_EXTRA_HPP
#include <string>
// Limpia la zona de entrada debajo del tablero
void limpiarZonaEntrada(int baseX, int baseY, int ancho);
// Imprime el recuadro de entrada debajo del tablero
void imprimirRecuadroEntrada(int baseX, int baseY, int ancho, const std::string& mensaje);
#endif
