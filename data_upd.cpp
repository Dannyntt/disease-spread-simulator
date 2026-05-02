#include "data_upd.h"
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

    for (char c : linea) {
        if (c == '"') {
            enComillas = !enComillas;
        } else if (c == sep && !enComillas) {
            // Trim espacios
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
}

// "DD/MM/YYYY", días desde 2020-03-06 
int fechaADias(const std::string& fecha) {
    // Tabla simple de meses para marzo-abril 2020
    if (fecha.size() < 8) return 0;
    try {
        int d = std::stoi(fecha.substr(0, 2));
        int m = std::stoi(fecha.substr(3, 2));
        int y = std::stoi(fecha.substr(6, 4));

        // Días acumulados
        int diasTotales = y * 365 + m * 30 + d;
        int origen      = 2020 * 365 + 3 * 30 + 6;  // 06/03/2020
        return diasTotales - origen;
    } catch (...) {
        return 0;
    }
}

// ── Construir grafo micro desde Casos1.csv ────────────────
Grafo cargarGrafoMicro(const std::string& pathCasos) {
    std::ifstream f(pathCasos);
    if (!f.is_open()) {
        std::cerr << "[ERROR] No se pudo abrir: " << pathCasos << "\n";
        return Grafo{};
    }

    Grafo G;

    std::string linea;
    std::getline(f, linea);
    // ID de caso, Fecha de diagnóstico, Ciudad de ubicación,
    // Departamento o Distrito, Atención**, Edad, Sexo, Tipo*, País de procedencia

    // Índice de importados por departamento: depto → lista de (id, dias)
    std::unordered_map<std::string, std::vector<std::pair<int,int>>> idxImport;
    // Todos los importados (para búsqueda global)
    std::vector<std::pair<int,int>> todosImport;

    // cargar todos los nodos
    std::vector<std::tuple<int,std::string,std::string,int>> relacionados;

    while (std::getline(f, linea)) {
        if (linea.empty()) continue;
        auto c = parsearLinea(linea);
        if (c.size() < 8) continue;

        Nodo n;
        try { n.id = std::stoi(c[0]); } catch (...) { continue; }

        n.fecha  = c[1];
        n.ciudad = c[2];
        n.depto  = c[3];
        // c[4] = Atención
        try { n.edad = std::stoi(c[5]); } catch (...) { n.edad = 0; }
        n.sexo   = c[6];
        n.tipo   = c[7];

        // Estado inicial: importados = I (semilla), resto = S
        n.estado = (n.tipo == "Importado") ? EstadoSIR::I : EstadoSIR::S;

        G.addNodo(n);

        int dias = fechaADias(n.fecha);

        if (n.tipo == "Importado") {
            idxImport[n.depto].push_back({n.id, dias});
            todosImport.push_back({n.id, dias});
        } else if (n.tipo == "Relacionado") {
            relacionados.push_back({n.id, n.depto, n.fecha, dias});
        }
    }

    //  conecta relacionados -> importado más cercano
    int aristasCreadas = 0;
    const int VENTANA_DIAS = 14;

    for (auto& [rid, depto, fecha, dias] : relacionados) {
        // Buscar en el mismo departamento primero
        auto& candidatos = idxImport.count(depto)
                           ? idxImport.at(depto) : todosImport;

        int mejorId   = -1;
        int menorDiff = INT_MAX;

        for (auto& [cid, cdias] : candidatos) {
            int diff = std::abs(dias - cdias);
            if (diff < menorDiff) {
                menorDiff = diff;
                mejorId   = cid;
            }
        }

        if (mejorId != -1 && menorDiff <= VENTANA_DIAS) {
            double peso = 1.0 / (1.0 + menorDiff);
            G.addArista(rid, mejorId, peso, menorDiff);
            ++aristasCreadas;
        }
    }

    std::cout << "[Grafo micro] Nodos: " << G.numNodos()
              << "  Aristas: " << G.numAristas()
              << "  (ventana epidemiologica: " << VENTANA_DIAS << " dias)\n";
    return G;
}

// Calibrar β y γ desde datos nacionales
ParametrosSIR calibrarParametros(const std::string& pathNacional) {
    // β = 0.25 (calibrado para red dispersa COVID primera ola)
    // γ = 1/14 ≈ 0.0714 (período infeccioso estándar COVID)
    // R₀ ≈ 3.5 — > segun los reportes de colombia

    std::ifstream f(pathNacional);
    double beta  = 0.25;
    double gamma = 1.0 / 14.0;

    if (f.is_open()) {
        std::string linea;
        std::getline(f, linea); // encabezado
        std::vector<int> confirmedDaily;
        while (std::getline(f, linea)) {
            auto c = parsearLinea(linea);
            if (c.size() >= 3) {
                try {
                    int cd = std::stoi(c[2]); // confirmed_daily
                    if (cd > 0) confirmedDaily.push_back(cd);
                } catch (...) {}
            }
        }
        // β estimado en fase exponencial
        if (!confirmedDaily.empty()) {
            double promDiario = 0;
            int lim = std::min((int)confirmedDaily.size(), 7);
            for (int i = 0; i < lim; ++i) promDiario += confirmedDaily[i];
            promDiario /= lim;
            // Ajuste empírico para la red dispersa
            beta = std::min(0.4, std::max(0.15, promDiario / 20.0));
        }
    }

    std::cout << "[Parametros SIR] beta=" << beta
              << "  gamma=" << gamma
              << "  R0=" << beta / gamma << "\n";
    return {beta, gamma};
}

} 