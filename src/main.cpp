#include <iostream>
#include <limits>
#include "GameBoard.hpp"
#include "Player.hpp"
#include "RecordManager.hpp"
#include "MultiplayerServer.hpp"
#include "MultiplayerClient.hpp"
#include "Utils.hpp"

AdministradorDeRecords administradorDeRecords;
const std::string ARCHIVO_RECORDS = "records.csv";

void mostrarMenuPrincipal();
void mostrarMenuIndividual();
void mostrarMenuMultijugador();
void mostrarRecords();
void mostrarAcercaDe();

void jugarIndividual(TableroJuego::Dificultad diff, int filasPersonalizadas = 0, int columnasPersonalizadas = 0, int minasPersonalizadas = 0);

int main(){
    administradorDeRecords.cargarDesdeArchivo(ARCHIVO_RECORDS);
    int opcion;
    bool salir = false;

    while (!salir) {
        mostrarMenuPrincipal();
        std::cin >> opcion;
        switch(opcion) {
            case 1: { // Individual
                int subOpcion = 0;
                do {
                    mostrarMenuIndividual();
                    std::cin >> subOpcion;
                    switch (subOpcion) {
                        case 1:
                            jugarIndividual(TableroJuego::PRINCIPIANTE);
                            break;
                        case 2:
                            jugarIndividual(TableroJuego::INTERMEDIO);
                            break;
                        case 3:
                            jugarIndividual(TableroJuego::EXPERTO);
                            break;
                        case 4: {
                            int filas = obtenerEntrada("Filas (5-30): ", 5, 30);
                            int columnas = obtenerEntrada("Columnas: (5-30): ", 5, 30);
                            int maxMinas = (filas * columnas) / 2;
                            int minas = obtenerEntrada("Minas (1-" + std::to_string(maxMinas) + "): ", 1, maxMinas);
                            jugarIndividual(TableroJuego::PERSONALIZADO, filas, columnas, minas);
                            break;
                        }
                        case 5:
                            break;
                        default:
                            std::cout << "Opción inválida!\n";
                            break;
                    }
                } while (subOpcion != 5);
                break;
            }
            case 2: { // Multijugador
                int mpOpcion = 0;
                do {
                    mostrarMenuMultijugador();
                    std::cin >> mpOpcion;
                    switch (mpOpcion) {
                        case 1: { // Crear Partida
                            int puerto = obtenerEntrada("Puerto (1024-65535): ", 1024, 65535);
                            int diffOpcion;
                            std::cout << "\n"
                                "      ____                                  _                 \n"
                                "     / __ )__  ________________ _____ ___  (_)___  ____ ______\n"
                                "    / __  / / / / ___/ ___/ __ `/ __ `__ \\/ / __ \\/ __ `/ ___/\n"
                                "   / /_/ / /_/ (__  ) /__/ /_/ / / / / / / / / / / / /_/ (__  ) \n"
                                "  /_____/\\__,_/____/\\___/\\__,_/_/ /_/ /_/_/_/ /_/\\__,_/____/  \n"
                                "                                                              \n";
                            std::cout << "1. Principiante\n";
                            std::cout << "2. Intermedio\n";
                            std::cout << "3. Experto\n";
                            std::cout << "4. Personalizado\n";
                            std::cin >> diffOpcion;
                            TableroJuego::Dificultad diff;
                            int filas = 8, columnas = 8, minas = 10;
                            switch (diffOpcion) {
                                case 1:
                                    diff = TableroJuego::PRINCIPIANTE;
                                    break;
                                case 2:
                                    diff = TableroJuego::INTERMEDIO;
                                    break;
                                case 3:
                                    diff = TableroJuego::EXPERTO;
                                    break;
                                case 4: {
                                    diff = TableroJuego::PERSONALIZADO;
                                    filas = obtenerEntrada("Filas (5-30): ", 5, 30);
                                    columnas = obtenerEntrada("Columnas: (5-30): ", 5, 30);
                                    int maxMinas = (filas * columnas) / 2;
                                    minas = obtenerEntrada("Minas (1-" + std::to_string(maxMinas) + "): ", 1, maxMinas);
                                    break;
                                }
                                default:
                                    std::cout << "Opcion inválida\n";
                                    continue;
                            }
                            ServidorMultijugador servidor(puerto, diff, filas, columnas, minas);
                            if(servidor.iniciar()) {
                                servidor.ejecutarJuego();
                            }
                            break;
                        }
                        case 2: { // Unirse a la partida
                            std::string ip;
                            int puerto;
                            std::cout << "IP del servidor: ";
                            std::cin >> ip;
                            puerto = obtenerEntrada("Puerto: ", 1024, 65535);
                            ClienteMultijugador cliente(ip, puerto);
                            if(cliente.conectar()) {
                                cliente.jugar();
                            }
                            break;
                        }
                        case 3:
                            break;
                        default:
                            std::cout << "Opción inválida!\n";
                            break;
                    }
                } while (mpOpcion != 3);
                break;
            }
            case 3:
                mostrarRecords();
                break;
            case 4:
                mostrarAcercaDe();
                break;
            case 5:
                salir = true;
                break;
            default:
                std::cout << "Opción inválida!\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
        }
    }
    return 0;
}

// Imprime una línea alineada dentro de un marco, compensando ancho visual
void printMenuLine(int baseX, int y, const std::string& content, int ancho) {
    gotoxy(baseX, y);
    std::cout << "│ " << content;
    // Calcular ancho visual (emojis y algunos caracteres valen 2)
    int visual = 2; // por el espacio después de │
    for (size_t i = 0; i < content.size(); ++i) {
        unsigned char c = content[i];
        if ((c & 0x80) == 0) visual += 1; // ASCII
        else if ((c & 0xE0) == 0xC0) { visual += 2; ++i; } // 2 bytes
        else if ((c & 0xF0) == 0xE0) { visual += 2; i+=2; } // 3 bytes (emoji)
        else if ((c & 0xF8) == 0xF0) { visual += 2; i+=3; } // 4 bytes (emoji)
    }
    // Calcula la posición absoluta de la barra derecha
    int barraX = baseX + ancho - 1;
    gotoxy(barraX, y);
    std::cout << "│\n";
}

void mostrarMenuPrincipal(){
    limpiarPantalla();
    int baseX = 10, baseY = 3;
    const int ancho = 46;
    gotoxy(baseX, baseY);
    std::cout << "\033[96m";
    std::cout << "┌";
    for(int i=0; i<ancho-2; ++i) std::cout << "─";
    std::cout << "┐\n";
    printMenuLine(baseX, baseY+1, "             BUSCAMINAS", ancho);
    gotoxy(baseX, baseY+2);
    std::cout << "├";
    for(int i=0; i<ancho-2; ++i) std::cout << "─";
    std::cout << "┤\n";
    printMenuLine(baseX, baseY+3,  "1. 🎲 Juego Individual", ancho);
    printMenuLine(baseX, baseY+4,  "2. 🏴 Multijugador", ancho);
    printMenuLine(baseX, baseY+5,  "3. 🏆 Records", ancho);
    printMenuLine(baseX, baseY+6,  "4. ℹ️  Acerca de", ancho);
    printMenuLine(baseX, baseY+7,  "5. ❌ Salir", ancho);
    gotoxy(baseX, baseY+8);
    std::cout << "└";
    for(int i=0; i<ancho-2; ++i) std::cout << "─";
    std::cout << "┘\n";
    // Recuadro para la opción
    int optX = baseX + 14, optY = baseY + 10;
    gotoxy(optX, optY);
    std::cout << "┌──────────────┐";
    gotoxy(optX, optY+1);
    std::cout << "│ Opción:      │";
    gotoxy(optX, optY+2);
    std::cout << "└──────────────┘";
    gotoxy(optX+9, optY+1);
    std::cout << "\033[0m";
}

void mostrarMenuIndividual(){
    limpiarPantalla();
    int baseX = 12, baseY = 5;
    const int ancho = 38;
    gotoxy(baseX, baseY);
    std::cout << "\033[92m";
    std::cout << "┌"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┐\n";
    printMenuLine(baseX, baseY+1, "        MODO INDIVIDUAL", ancho);
    gotoxy(baseX, baseY+2);
    std::cout << "├"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┤\n";
    printMenuLine(baseX, baseY+3, "1. Principiante (8x8 - 10 minas)", ancho);
    printMenuLine(baseX, baseY+4, "2. Intermedio (16x16 - 40 minas)", ancho);
    printMenuLine(baseX, baseY+5, "3. Experto (30x16 - 99 minas)", ancho);
    printMenuLine(baseX, baseY+6, "4. Personalizado", ancho);
    printMenuLine(baseX, baseY+7, "5. Volver al menú principal", ancho);
    gotoxy(baseX, baseY+8);
    std::cout << "└"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┘\n";
    // El recuadro de opción se elimina para dejar espacio al recuadro de nombre
    std::cout << "\033[0m";
}

void mostrarMenuMultijugador(){
    limpiarPantalla();
    int baseX = 12, baseY = 5;
    const int ancho = 42;
    gotoxy(baseX, baseY);
    std::cout << "\033[94m";
    std::cout << "┌"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┐\n";
    printMenuLine(baseX, baseY+1, "      MODO MULTIJUGADOR", ancho);
    gotoxy(baseX, baseY+2);
    std::cout << "├"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┤\n";
    printMenuLine(baseX, baseY+3, "1. Crear partida (servidor)", ancho);
    printMenuLine(baseX, baseY+4, "2. Unirse a partida (cliente)", ancho);
    printMenuLine(baseX, baseY+5, "3. Volver al menú principal", ancho);
    gotoxy(baseX, baseY+6);
    std::cout << "└"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┘\n";
    // Recuadro para la opción
    int optX = baseX + 12, optY = baseY + 8;
    gotoxy(optX, optY);
    std::cout << "┌──────────────┐";
    gotoxy(optX, optY+1);
    std::cout << "│ Opción:      │";
    gotoxy(optX, optY+2);
    std::cout << "└──────────────┘";
    gotoxy(optX+9, optY+1);
    std::cout << "\033[0m";
}

void mostrarRecords(){
    limpiarPantalla();
    int baseX = 8, baseY = 2;
    const int ancho = 62;
    gotoxy(baseX, baseY);
    std::cout << "\033[93m";
    // Marco superior
    std::cout << "┌"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┐\n";
    // Título
    printMenuLine(baseX, baseY+1, "                        TOP RECORDS", ancho);
    // Separador
    gotoxy(baseX, baseY+2);
    std::cout << "├"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┤\n";
    // Cuerpo de records
    auto records = administradorDeRecords.obtenerRecords();
    size_t maxRecords = std::min(records.size(), size_t(10));
    for(size_t i = 0; i < maxRecords; i++){
        const auto& r = records[i];
        std::string linea = std::to_string(i+1) + ". " + r.nombreJugador + " - " + dificultadAString(r.dificultad)
            + " - Punt: " + std::to_string(static_cast<int>(r.minas * 1000 / r.tiempo))
            + " - Tiempo: " + std::to_string(r.tiempo) + "s";
        printMenuLine(baseX, baseY+3+i, linea, ancho);
    }
    // Rellenar líneas vacías
    for(size_t i = maxRecords; i < 10; i++) {
        printMenuLine(baseX, baseY+3+i, "", ancho);
    }
    // Marco inferior
    gotoxy(baseX, baseY+13);
    std::cout << "└"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┘\n";
    gotoxy(baseX, baseY+15);
    std::cout << "\033[0mPresiona Enter para continuar...";
    std::cin.ignore();
    std::cin.get();
}

void jugarIndividual(TableroJuego::Dificultad diff, int filasPersonalizadas, int columnasPersonalizadas, int minasPersonalizadas){
    // Recuadro alineado para ingresar el nombre (en el recuadro de opción, pero más largo y debajo del menú)
    std::string nombre;
    int baseX = 12, baseY = 15; // Debajo del menú individual
    const int ancho = 38; // Igual que el menú de modo individual
    std::cout << "\033[95m";
    // Marco superior
    gotoxy(baseX, baseY);
    std::cout << "┌"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┐\n";
    // Línea de input con texto
    printMenuLine(baseX, baseY+1, "Ingrese su nombre:", ancho);
    // Línea de input vacío
    printMenuLine(baseX, baseY+2, "", ancho);
    // Marco inferior
    gotoxy(baseX, baseY+3);
    std::cout << "└"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┘\n";
    // Colocar el cursor para el input dentro del recuadro
    gotoxy(baseX+2, baseY+2);
    std::cout << "\033[0m";
    std::cin >> nombre;
    std::cout << "\033[95m";
    std::cout << "\033[0m";

    Jugador jugador(nombre);
    jugador.iniciarJuego(diff, filasPersonalizadas, columnasPersonalizadas, minasPersonalizadas);

    while(jugador.estaVivo() && !jugador.haGanado()){
        jugador.obtenerTablero().imprimirTablero();
        // Recuadro alineado para ingresar fila
        int baseInputX = 18, baseInputY = 8;
        const int anchoInput = 28;
        std::cout << "\033[96m";
        gotoxy(baseInputX, baseInputY);
        std::cout << "┌"; for(int i=0; i<anchoInput-2; ++i) std::cout << "─"; std::cout << "┐\n";
        printMenuLine(baseInputX, baseInputY+1, "Ingrese Fila:", anchoInput);
        printMenuLine(baseInputX, baseInputY+2, "", anchoInput);
        gotoxy(baseInputX, baseInputY+3);
        std::cout << "└"; for(int i=0; i<anchoInput-2; ++i) std::cout << "─"; std::cout << "┘\n";
        gotoxy(baseInputX+2, baseInputY+2);
        std::cout << "\033[0m";
        int fila = obtenerEntrada("", 0, jugador.obtenerTablero().obtenerFilas()-1);

        // Recuadro alineado para ingresar columna
        std::cout << "\033[96m";
        gotoxy(baseInputX, baseInputY+5);
        std::cout << "┌"; for(int i=0; i<anchoInput-2; ++i) std::cout << "─"; std::cout << "┐\n";
        printMenuLine(baseInputX, baseInputY+6, "Ingrese Columna:", anchoInput);
        printMenuLine(baseInputX, baseInputY+7, "", anchoInput);
        gotoxy(baseInputX, baseInputY+8);
        std::cout << "└"; for(int i=0; i<anchoInput-2; ++i) std::cout << "─"; std::cout << "┘\n";
        gotoxy(baseInputX+2, baseInputY+7);
        std::cout << "\033[0m";
        int columna = obtenerEntrada("", 0, jugador.obtenerTablero().obtenerColumnas()-1);

        // Recuadro alineado para acción
        std::cout << "\033[96m";
        gotoxy(baseInputX, baseInputY+10);
        std::cout << "┌"; for(int i=0; i<anchoInput-2; ++i) std::cout << "─"; std::cout << "┐\n";
        printMenuLine(baseInputX, baseInputY+11, "[D]estapar o [B]andera?", anchoInput);
        printMenuLine(baseInputX, baseInputY+12, "", anchoInput);
        gotoxy(baseInputX, baseInputY+13);
        std::cout << "└"; for(int i=0; i<anchoInput-2; ++i) std::cout << "─"; std::cout << "┘\n";
        gotoxy(baseInputX+2, baseInputY+12);
        std::cout << "\033[0m";
        char accion;
        std::cin >> accion;
        if(accion == 'B' || accion == 'b'){
            jugador.alternarBanderas(fila, columna);
        }else{
            jugador.realizarMovimiento(fila, columna);
        }
    }

    //Guardar record
    Record nuevoRecord{
        jugador.obtenerNombre(), diff, jugador.obtenerTablero().obtenerFilas(), jugador.obtenerTablero().obtenerColumnas(), jugador.obtenerTablero().obtenerMinas(), jugador.obtenerTablero().obtenerCeldasDestapadas(), jugador.obtenerTiempo(), jugador.haGanado(), obtenerFechaHoraActual()
    };

    administradorDeRecords.agregarRecord(nuevoRecord);
    administradorDeRecords.guardarEnArchivo(ARCHIVO_RECORDS);

    //Mostrar resultado final
    jugador.obtenerTablero().imprimirTablero();
    std::cout << "\n\n";
    std::cout << (jugador.haGanado() ? "!GANASTE¡ :)" : "!PERDISTE¡ :(") << "\n";
    std::cout << "Puntuacion: " << jugador.obtenerPuntuacion() << "\n";
    std::cout << "Presiona Enter para continuar...";
    std::cin.ignore();
    std::cin.get();
}

void mostrarAcercaDe(){
    limpiarPantalla();
    int baseX = 8, baseY = 4;
    const int ancho = 62;
    gotoxy(baseX, baseY);
    std::cout << "\033[96m";
    // Marco superior
    std::cout << "┌"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┐\n";
    // Título
    printMenuLine(baseX, baseY+1, "                        ACERCA DE", ancho);
    // Separador
    gotoxy(baseX, baseY+2);
    std::cout << "├"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┤\n";
    // Líneas de contenido
    printMenuLine(baseX, baseY+3, "Desarrollado por: Bryan Salazar y Tairon Rojas", ancho);
    printMenuLine(baseX, baseY+4, "(Los rebeldes de software)", ancho);
    printMenuLine(baseX, baseY+5, "Este juego está basado en un buscaminas con soporte", ancho);
    printMenuLine(baseX, baseY+6, "tanto para un jugador como multijugador en red local.", ancho);
    printMenuLine(baseX, baseY+7, "Desarrollado en C++", ancho);
    printMenuLine(baseX, baseY+8, "Creado en el año 2025", ancho);
    // Marco inferior
    gotoxy(baseX, baseY+9);
    std::cout << "└"; for(int i=0; i<ancho-2; ++i) std::cout << "─"; std::cout << "┘\n";
    gotoxy(baseX, baseY+11);
    std::cout << "\033[0mPresione Enter para continuar...";
    std::cin.ignore();
    std::cin.get();
}