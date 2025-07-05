#ifndef RECORDMANAGER_HPP
#define RECORDMANAGER_HPP

#include <vector>
#include <string>
#include "GameBoard.hpp"

struct Record {
    std::string nombreJugador;
    TableroJuego::Dificultad dificultad;
    int filas, columnas, minas, celdasDestapadas;
    double tiempo;
    bool gano;
    std::string fecha;
};

class AdministradorDeRecords {
    private:
    std::vector<Record>records;

    public:
    void agregarRecord(const Record& record);
    void guardarEnArchivo(const std::string& nombreArchivo);
    void cargarDesdeArchivo(const std::string& nombreArchivo);
    std::vector<Record>obtenerRecords() const;
};

#endif