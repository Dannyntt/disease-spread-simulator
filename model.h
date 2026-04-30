#pragma once
#include "graph.h"
#include <vector>
#include <string>
#include <random>

struct PuntoSIR {
    int tick;
    int S, I, R;
};

//  Simulador SIR sobre un Grafo
//  En cada tick:
//    - Nodo I intenta infectar vecinos S con probabilidad del coeficienta b * peso_arista
//    - Nodo I se recupera con prob γ
//  Parámetros:
//    β (beta) : tasa de contagio por contacto [0, 1]
//    γ (gamma): tasa de recuperación diaria  [0, 1]
//    R₀: número reproductivo≈ β/γ

class SimuladorSIR {
public:
    SimuladorSIR(double beta, double gamma, unsigned seed = 42);
    std::vector<PuntoSIR> simular(Grafo& G, int pasos);

    double beta()  const { return beta_;  }
    double gamma() const { return gamma_; }
    double R0()    const { return beta_ / gamma_; }

private:
    double beta_;
    double gamma_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;

    bool ocurre(double prob) { return dist_(rng_) < prob; }
};


namespace Estrategias {
    Grafo ninguna(const Grafo& G); //sin intervención, retorna al grafo original
    Grafo aislarHubs(const Grafo& G, double porcentaje = 0.05);
    Grafo rastreaContacnosBFS(const Grafo& G); //aisla aristas
    Grafo vacunacionDirigida(const Grafo& G, double porcentaje = 0.10);
    std::string nombre(const std::string& clave);
}