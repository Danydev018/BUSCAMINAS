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
    // Marco superior estilo tablero
    printf("\033[%d;%dH", baseY, baseX);
    std::cout << "┌";
    for(int i=0; i<ancho-2; ++i) std::cout << "─";
    std::cout << "┐";
    // Línea vacía
    printf("\033[%d;%dH", baseY+1, baseX);
    std::cout << "│" << std::string(ancho-2, ' ') << "│";
    // Mensaje y espacio para input justo después
    printf("\033[%d;%dH", baseY+2, baseX);
    std::cout << "│" << mensaje;
    int cursorX = baseX + 1 + (int)mensaje.length();
    int cursorY = baseY + 2;
    std::cout << std::string(ancho-2-mensaje.length(), ' ') << "│";
    // Marco inferior
    printf("\033[%d;%dH", baseY+3, baseX);
    std::cout << "└";
    for(int i=0; i<ancho-2; ++i) std::cout << "─";
    std::cout << "┘";
    // Coloca el cursor justo después del mensaje
    printf("\033[%d;%dH", cursorY, cursorX);
}
