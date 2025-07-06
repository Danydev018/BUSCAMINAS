#include "../include/UtilsExtra.hpp"
#include <iostream>
#include <string>
#include <cstdio>
// Limpia la zona de entrada debajo del tablero
void limpiarZonaEntrada(int baseX, int baseY, int ancho) {
    for (int i = 0; i < 4; ++i) {
        printf("\033[%d;%dH", baseY + i, baseX);
        std::cout << std::string(ancho+10, ' ');
    }
}
// Imprime el recuadro de entrada debajo del tablero
void imprimirRecuadroEntrada(int baseX, int baseY, int ancho, const std::string& mensaje) {
    limpiarZonaEntrada(baseX, baseY, ancho);
    printf("\033[%d;%dH", baseY, baseX);
    std::cout << "+" << std::string(ancho-2, '-') << "+";
    printf("\033[%d;%dH", baseY+1, baseX);
    std::cout << "|" << std::string(ancho-2, ' ') << "|";
    printf("\033[%d;%dH", baseY+2, baseX);
    std::cout << "|" << mensaje << std::string(ancho-2-mensaje.length(), ' ') << "|";
    printf("\033[%d;%dH", baseY+3, baseX);
    std::cout << "+" << std::string(ancho-2, '-') << "+";
}
