#include "../include/RecordManager.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include "../include/Utils.hpp"


void AdministradorDeRecords::agregarRecord(const Record& record){
    records.push_back(record);
}

void AdministradorDeRecords::guardarEnArchivo(const std::string& nombreArchivo){
    std::ofstream
    archivo(nombreArchivo);
    if(!archivo.is_open())
     return;

     // Encabezado CSV
     archivo <<"Nombre, Dificultad, Filas, Columnas, Minas, CeldasDestapadas, Tiempo, Ganado, Fecha\n";

     for(const auto& record : records){
         archivo << record.nombreJugador << ","  << dificultadAString(record.dificultad) << ","
                 << record.filas << "," <<record.columnas << "," << record.minas << ","
                 << record.celdasDestapadas << "," << record.tiempo << "," << (record.gano ? "SI" : "NO") << ","
                 << record.fecha << "\n";
        }
}

void AdministradorDeRecords::cargarDesdeArchivo(const std::string& nombreArchivo){
    std::ifstream
    archivo(nombreArchivo);
    if(!archivo.is_open())
    return;

    records.clear();
    std::string linea;
    std::getline(archivo, linea); //Saltar encabezado

    while(std::getline(archivo, linea)){
        std::stringstream ss(linea);
        Record record;
        std::string token;

        //Parsear cada cmpo usando comas (,) como delimitadores
        std::getline(ss, token, ',');
        record.nombreJugador = token;

        std::getline(ss, token, ',');
        if(token == "Principiante") record.dificultad = TableroJuego::PRINCIPIANTE;
        else if(token == "Intermedio") record.dificultad = TableroJuego::INTERMEDIO;
        else if(token == "Experto") record.dificultad = TableroJuego::EXPERTO;
        else record.dificultad = TableroJuego::PERSONALIZADO;

        std::getline(ss, token, ',');
        record.filas = std::stoi(token);

        std::getline(ss, token, ',');
        record.columnas = std::stoi(token);

        std::getline(ss, token, ',');
        record.minas = std::stoi(token);

        std::getline(ss, token, ',');
        record.celdasDestapadas = std::stoi(token);

        std::getline(ss, token, ',');
        record.tiempo = std::stod(token);

        std::getline(ss, token, ',');
        record.gano = (token == "Si") ;

        std::getline(ss, token, ',');
        record.fecha = (token);

        records.push_back(record);

    }
}

std::vector<Record> AdministradorDeRecords::obtenerRecords() const{
    //Ordenar puntuacion (minas/tiempo)
    std::vector<Record> copia = records;
    std::sort(copia.begin(), copia.end(), [](const Record& a, const Record& b){
        return (a.minas / a.tiempo) > (b.minas / b.tiempo);
    });
    return copia;
}
