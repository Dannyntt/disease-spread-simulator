#ifndef DATA_UPD_H
#define DATA_UPD_H

#include <string>
#include <vector>

#include "graph.h"

namespace CargaDatos {

struct ParametrosSIR {
    double beta;
    double gamma;

    double R0() const {
        return beta / gamma;
    }
};

std::vector<std::string> parsearLinea(const std::string& linea, char sep = ',');

int fechaADias(const std::string& fecha);

Grafo cargarGrafoMicro(const std::string& pathCasos);

ParametrosSIR calibrarParametros(const std::string& pathNacional);

}

#endif 
