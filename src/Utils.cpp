#include "../include/Utils.hpp"
#include "GameBoard.hpp"
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstddef>

std::string obtenerFechaHoraActual(){
    auto ahora = std::chrono::system_clock::now();
    auto en_tiempo_t = std::chrono::system_clock::to_time_t(ahora);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&en_tiempo_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void limpiarPantalla(){
    #ifdef _WIN32
      std::system("cls");
    #else
      std::system("clear");
    #endif
}

int obtenerEntrada(const std::string& mensaje, int min, int max){
    int valor;
    while(true){
        std::cout << mensaje;
        if(std::cin >> valor && valor >= min && valor <= max){
            return valor;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout <<"Entrada inválida. Intente nuevamente.\n";
    }
}

std::string dificultadAString(TableroJuego::Dificultad diff){
    switch(diff){
        case
        TableroJuego::PRINCIPIANTE: return "Principiante";
        case
        TableroJuego::INTERMEDIO: return "Intermedio";
        case
        TableroJuego::EXPERTO: return "Experto";
        case TableroJuego::PERSONALIZADO: return "Personalizado";
        default: return "Error desconocido";
    }
}

// Imprime una línea alineada dentro de un marco, compensando ancho visual (para tablero)
void printBoardLine(int baseX, int y, const std::string& content, int ancho) {
    gotoxy(baseX, y);
    std::cout << "│" << content;
    // Calcular ancho visual (emojis y algunos caracteres valen 2)
    int visual = 0;
    for (size_t i = 0; i < content.size(); ++i) {
        unsigned char c = content[i];
        if ((c & 0x80) == 0) visual += 1; // ASCII
        else if ((c & 0xE0) == 0xC0) { visual += 2; ++i; } // 2 bytes
        else if ((c & 0xF0) == 0xE0) { visual += 2; i+=2; } // 3 bytes (emoji)
        else if ((c & 0xF8) == 0xF0) { visual += 2; i+=3; } // 4 bytes (emoji)
    }
    int barraX = baseX + ancho - 1;
    gotoxy(barraX, y);
    std::cout << "│\n";
}

// Implementación de gotoxy para mover el cursor en la terminal
void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}
