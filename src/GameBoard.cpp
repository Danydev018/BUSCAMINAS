#include "../include/GameBoard.hpp"
#include "../include/Utils.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

TableroJuego::TableroJuego(Dificultad diff, int filasPersonalizadas, int columnasPersonalizadas, int minasPersonalizadas) : dificultad(diff), celdasDestapadas(0), banderasColocadas(0), juegoTerminado(false), juegoGanado(false){
    switch(diff){
        case PRINCIPIANTE:
        filas = 8; columnas = 8; minas = 10;
        break;

        case INTERMEDIO:
        filas = 16; columnas = 16; minas = 40;
        break;

        case EXPERTO:
        filas = 30; columnas = 16; minas = 99;
        break;

        case PERSONALIZADO:
        filas = filasPersonalizadas; columnas = columnasPersonalizadas; minas = minasPersonalizadas;
        break; 
    }

    //Inicializar tablero
    tableroVisible.resize(filas, std::vector<char>(columnas, '-'));
    tableroMinas.resize(filas, std::vector<bool>(columnas, false));
    tableroAdyacentes.resize(filas, std::vector<int>(columnas, 0));
}

void TableroJuego::inicializar(){
    // Limpiar tableros
    for(int i = 0; i < filas; i++){
        std::fill(tableroVisible[i].begin(), tableroVisible[i].end(), '-');
        std::fill(tableroMinas[i].begin(), tableroMinas[i].end(), false);
        std::fill(tableroAdyacentes[i].begin(), tableroAdyacentes[i].end(), 0);
    }

    celdasDestapadas = 0;
    banderasColocadas = 0;
    juegoTerminado = false;
    juegoGanado = false;

    colocarMinas();
    calcularMinasAdyacentes();
    iniciarCronometro();
}

void TableroJuego::colocarMinas(){
    srand(time(0));
    int minasColocadas = 0;

    while(minasColocadas < minas){
        int f = rand() % filas;
        int c = rand() % columnas;

        if(!tableroMinas[f][c]){
            tableroMinas[f][c] = true;
            minasColocadas++;
        }
    }
}

void TableroJuego::calcularMinasAdyacentes(){
    for(int f = 0; f < filas; f++){
        for(int c = 0; c < columnas; c++){
            if(!tableroMinas[f][c]){
                int contador = 0;
                for(int df = -1; df <= 1; df++){
                    for(int dc = -1; dc <= 1; dc++){
                        int nf = f + df;
                        int nc = c + dc;
                        if(esPosicionValida(nf, nc) && tableroMinas[nf][nc]){
                            contador++;
                        }
                    }
                }
                tableroAdyacentes[f][c] = contador;
            }
        }
    }
}

bool TableroJuego::destapar(int fila, int columna){
    if(!esPosicionValida(fila, columna) || tableroVisible[fila][columna] != '-' || juegoTerminado)
    return false;

    if(tableroMinas[fila][columna]){
        juegoTerminado = true;
        tableroVisible[fila][columna] = 'X'; //Mina explotada
        return true;
    }

    if(tableroAdyacentes[fila][columna] > 0){
        tableroVisible[fila][columna] = '0' + tableroAdyacentes[fila][columna];
        celdasDestapadas++;
    }else{
        //Revelar área vacía recursivamente
        revelarCeldasAdyacentes(fila, columna);
    }

    //Verificar si ganó
    if((filas * columnas - celdasDestapadas) == minas){
        juegoGanado = true;
        detenerCronometro();
    }
    return true;
}

void TableroJuego::revelarCeldasAdyacentes(int fila, int columna){
    if(!esPosicionValida(fila, columna) || tableroVisible[fila][columna] != '-')
    return;

    if(tableroAdyacentes[fila][columna] > 0){
        tableroVisible[fila][columna] = '0' + tableroAdyacentes[fila][columna];
        celdasDestapadas++;
        return;
    }
    tableroVisible[fila][columna] = ' ';
    celdasDestapadas++;

    //Revear vecinos recursivamente
    for(int df = -1; df <= 1; df++){
        for(int dc = -1; dc <= 1; dc++){
            if(df == 0 && dc == 0) continue;

            revelarCeldasAdyacentes(fila + df, columna + dc);
        }
    }
}

void TableroJuego::alternarBandera(int fila, int columna){
    if(!esPosicionValida(fila, columna) || juegoTerminado)
    return;

    if(tableroVisible[fila][columna] == '-'){
        tableroVisible[fila][columna] = 'F';
        banderasColocadas++;
    }else if(tableroVisible[fila][columna] == 'F'){
        tableroVisible[fila][columna] = '-';
        banderasColocadas--;
    }
}

void TableroJuego::imprimirTablero() const {
    limpiarPantalla();
    // Colores ANSI y emojis para consola
    int baseX = 8, baseY = 2;
    int cellW = 5, cellH = 1; // cada celda mide 5x1 para centrar emojis y evitar distorsión
    int ancho = columnas * cellW + 1;
    int alto = filas * cellH + 1;
    const char* RESET = "\033[0m";
    const char* GRIS = "\033[90m";
    const char* ROJO = "\033[91m";
    const char* VERDE = "\033[92m";
    const char* AMARILLO = "\033[93m";
    const char* AZUL = "\033[94m";
    const char* CYAN = "\033[96m";
    const char* BLANCO = "\033[97m";

    std::string emoji_bomba = "\xF0\x9F\x92\xA3"; // 💣
    std::string emoji_bandera = "\xF0\x9F\x9A\xA9"; // 🚩
    std::string emoji_oculto = "■";

    // Imprimir encabezado de columnas centrado en cada casilla
    for (int c = 0; c < columnas; ++c) {
        std::string colStr = std::to_string(c);
        int colX = baseX + 4 + c * (cellW + 1) + cellW / 2 - (int)colStr.length() / 2;
        int colY = baseY;
        gotoxy(colX, colY);
        std::cout << colStr;
    }

    // Marco superior
    gotoxy(baseX, baseY+1);
    std::cout << "   ┌";
    for (int c = 0; c < columnas-1; ++c) {
        for (int i = 0; i < cellW; ++i) std::cout << "─";
        std::cout << "┬";
    }
    for (int i = 0; i < cellW; ++i) std::cout << "─";
    std::cout << "┐";

    // Imprimir solo el marco y los separadores de las filas
    for (int f = 0; f < filas; ++f) {
        // Número de fila y marco izquierdo
        gotoxy(baseX, baseY+2+f*2);
        std::cout << (f < 10 ? " " : "") << f << " │";
        // Imprimir solo los separadores de celdas
        for (int c = 0; c < columnas; ++c) {
            for (int i = 0; i < cellW; ++i) std::cout << " ";
            std::cout << "│";
        }
        // Línea separadora (excepto la última)
        if (f < filas-1) {
            gotoxy(baseX, baseY+3+f*2);
            std::cout << "   ├";
            for (int c = 0; c < columnas-1; ++c) {
                for (int i = 0; i < cellW; ++i) std::cout << "─";
                std::cout << "┼";
            }
            for (int i = 0; i < cellW; ++i) std::cout << "─";
            std::cout << "┤";
        }
    }

    // Imprimir el contenido de cada casilla centrado, encima del marco
    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            char celda = tableroVisible[f][c];
            std::string contenido;
            if(celda == '-') contenido = GRIS + emoji_oculto + RESET;
            else if(celda == 'F') contenido = ROJO + emoji_bandera + RESET;
            else if(celda == 'X' || celda == '*') contenido = ROJO + emoji_bomba + RESET;
            else if(celda >= '1' && celda <= '8') {
                const char* color = BLANCO;
                if(celda == '1') color = AZUL;
                else if(celda == '2') color = VERDE;
                else if(celda == '3') color = ROJO;
                else if(celda == '4') color = CYAN;
                else if(celda == '5') color = AMARILLO;
                else if(celda == '6') color = GRIS;
                else if(celda == '7') color = BLANCO;
                else if(celda == '8') color = BLANCO;
                contenido = std::string(color) + celda + RESET;
            } else if(celda == '0' || celda == ' ') contenido = " ";
            else contenido = BLANCO + std::string(1, celda) + RESET;
            // Calcular posición centrada en la casilla
            int cellX = baseX + 4 + c * (cellW + 1); // +4 por el número de fila y marco
            int cellY = baseY + 2 + f * 2;
            int left = cellW / 2;
            gotoxy(cellX + left, cellY);
            std::cout << contenido;
        }
    }
    // Marco inferior
    gotoxy(baseX, baseY+1+filas*2);
    std::cout << "   └";
    for (int c = 0; c < columnas-1; ++c) {
        for (int i = 0; i < cellW; ++i) std::cout << "─";
        std::cout << "┴";
    }
    for (int i = 0; i < cellW; ++i) std::cout << "─";
    std::cout << "┘";

    // Info debajo del tablero
    gotoxy(baseX, baseY+3+filas*2);
    std::cout << "Minas: " << minas << "  Banderas: " << banderasColocadas << "  Destapadas: " << celdasDestapadas << "  Tiempo: " << (int)obtenerTiempoTranscurrido() << "s";
    gotoxy(baseX, baseY+4+filas*2);
    std::cout << "Leyenda: " << GRIS << emoji_oculto << RESET << "=Oculto  " << ROJO << emoji_bandera << RESET << "=Bandera  " << ROJO << emoji_bomba << RESET << "=Mina";
    // Deja espacio para el recuadro de entrada
    gotoxy(baseX, baseY+6+filas*2);
    std::cout << std::string(ancho+10, ' ');

    // Sugerencia de uso para el recuadro de entrada:
    // #include "../include/UtilsExtra.hpp"
    // imprimirRecuadroEntrada(baseX, baseY+6+filas*2, ancho, "Ingrese fila, columna y acción (D/B): ");
    // limpiarZonaEntrada(baseX, baseY+6+filas*2, ancho); // para limpiar antes de mostrar otro recuadro
}

bool TableroJuego::esJuegoTerminado() const {
    return juegoTerminado;
}

bool TableroJuego::esJuegoGanado() const {
    return juegoGanado;
}

int TableroJuego::obtenerCeldasDestapadas() const{
    return celdasDestapadas;
}

double TableroJuego::obtenerTiempoTranscurrido() const{
    if(juegoTerminado || juegoGanado){
        return difftime(tiempoFin, tiempoInicio);
    }else{
        return difftime(time(nullptr), tiempoInicio); 
    }
}

void TableroJuego::iniciarCronometro(){
    tiempoInicio = time(nullptr);
}

void TableroJuego::detenerCronometro(){
    tiempoFin = time(nullptr);
}

int TableroJuego::obtenerFilas() const{
    return filas;
}

int TableroJuego::obtenerColumnas() const{
    return columnas;
}

int TableroJuego::obtenerMinas() const{
    return minas;
}

bool TableroJuego::esPosicionValida(int fila, int columna) const{
    return (fila >= 0 && fila < filas && columna >= 0 && columna < columnas);
}

