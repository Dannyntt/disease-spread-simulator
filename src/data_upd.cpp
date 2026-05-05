#include “data_upd.h”
#include “graph.h”          // <– necesario para Grafo, Nodo, EstadoSIR
#include <vector>           // <– necesario para std::vector
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_map>
#include <climits>

namespace CargaDatos {

std::vector<std::string> parsearLinea(const std::string& linea, char sep) {
std::vector<std::string> campos;
std::string campo;
bool enComillas = false;

```
for (char c : linea) {
    if (c == '"') {
        enComillas = !enComillas;
    } else if (c == sep && !enComillas) {
        auto inicio = campo.find_first_not_of(" \t\r");
        auto fin    = campo.find_last_not_of(" \t\r");
        campos.push_back(
            (inicio == std::string::npos) ? "" :
            campo.substr(inicio, fin - inicio + 1)
        );
        campo.clear();
    } else {
        campo += c;
    }
}
campos.push_back(campo);
return campos;
```

}

// “DD/MM/YYYY”, días desde 2020-03-06
int fechaADias(const std::string& fecha) {
if (fecha.size() < 8) return 0;
try {
int d = std::stoi(fecha.substr(0, 2));
int m = std::stoi(fecha.substr(3, 2));
int y = std::stoi(fecha.substr(6, 4));
int diasTotales = y * 365 + m * 30 + d;
int origen      = 2020 * 365 + 3 * 30 + 6;  // 06/03/2020
return diasTotales - origen;
} catch (…) {
return 0;
}
}

// ── Construir grafo micro desde Casos1.csv ────────────────────
Grafo cargarGrafoMicro(const std::string& pathCasos) {
std::ifstream f(pathCasos);
if (!f.is_open()) {
std::cerr << “[ERROR] No se pudo abrir: “ << pathCasos << “\n”;
return Grafo{};
}

```
Grafo G;
std::string linea;
std::getline(f, linea); // encabezado
// Columnas: ID, Fecha diagnóstico, Ciudad, Departamento,
//           Atención, Edad, Sexo, Tipo, País procedencia

std::unordered_map<std::string, std::vector<std::pair<int,int>>> idxImport;
std::vector<std::pair<int,int>> todosImport;
std::vector<std::tuple<int,std::string,std::string,int>> relacionados;

while (std::getline(f, linea)) {
    if (linea.empty()) continue;
    auto c = parsearLinea(linea, ',');
    if (c.size() < 8) continue;

    Nodo n;
    try { n.id = std::stoi(c[0]); } catch (...) { continue; }

    // ── CORRECCIÓN: n.campo → n.meta.campo ───────────────
    n.meta.fecha  = c[1];
    n.meta.ciudad = c[2];
    n.meta.depto  = c[3];
    // c[4] = Atención (no se almacena)
    try { n.meta.edad = std::stoi(c[5]); } catch (...) { n.meta.edad = 0; }
    n.meta.sexo   = c[6];
    n.meta.tipo   = c[7];

    // Importados = semilla (I), resto = susceptibles (S)
    n.estado = (n.meta.tipo == "Importado") ? EstadoSIR::I : EstadoSIR::S;

    G.addNodo(n);

    int dias = fechaADias(n.meta.fecha);

    if (n.meta.tipo == "Importado") {
        idxImport[n.meta.depto].push_back({n.id, dias});
        todosImport.push_back({n.id, dias});
    } else if (n.meta.tipo == "Relacionado") {
        relacionados.push_back({n.id, n.meta.depto, n.meta.fecha, dias});
    }
}

// Conectar relacionados → importado más cercano en tiempo
const int VENTANA_DIAS = 14;

for (auto& [rid, depto, fecha, dias] : relacionados) {
    auto& candidatos = idxImport.count(depto)
                       ? idxImport.at(depto) : todosImport;

    int mejorId   = -1;
    int menorDiff = INT_MAX;

    for (auto& [cid, cdias] : candidatos) {
        int diff = std::abs(dias - cdias);
        if (diff < menorDiff) { menorDiff = diff; mejorId = cid; }
    }

    if (mejorId != -1 && menorDiff <= VENTANA_DIAS) {
        double peso = 1.0 / (1.0 + menorDiff);
        G.addArista(rid, mejorId, peso, menorDiff);
    }
}

std::cout << "[Grafo micro] Nodos: " << G.numNodos()
          << "  Aristas: " << G.numAristas()
          << "  (ventana epidemiologica: " << VENTANA_DIAS << " dias)\n";
return G;
```

}

// ── Calibrar β y γ desde datos nacionales ────────────────────
ParametrosSIR calibrarParametros(const std::string& pathNacional) {
double beta  = 0.25;
double gamma = 1.0 / 14.0;

```
std::ifstream f(pathNacional);
if (f.is_open()) {
    std::string linea;
    std::getline(f, linea); // encabezado
    std::vector<int> confirmedDaily;
    while (std::getline(f, linea)) {
        auto c = parsearLinea(linea, ',');
        if (c.size() >= 3) {
            try {
                int cd = std::stoi(c[2]);
                if (cd > 0) confirmedDaily.push_back(cd);
            } catch (...) {}
        }
    }
    if (!confirmedDaily.empty()) {
        double promDiario = 0;
        int lim = std::min((int)confirmedDaily.size(), 7);
        for (int i = 0; i < lim; ++i) promDiario += confirmedDaily[i];
        promDiario /= lim;
        beta = std::min(0.4, std::max(0.15, promDiario / 20.0));
    }
}

std::cout << "[Parametros SIR] beta=" << beta
          << "  gamma=" << gamma
          << "  R0=" << beta / gamma << "\n";
return {beta, gamma};
```

}

} // namespace CargaDatos