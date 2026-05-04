#pragma once

#include "graph.h"
#include <vector>
#include <string>
#include <random>

// ─────────────────────────────────────────────────────────────
//  PuntoSIR: snapshot del estado de la población en un tick
// ─────────────────────────────────────────────────────────────
struct PuntoSIR {
    int tick;
    int S, I, R;
};

// ─────────────────────────────────────────────────────────────
//  SimuladorSIR: simula la propagación sobre un Grafo
//
//  En cada tick:
//    - Nodo I intenta infectar vecinos S con prob β * peso_arista
//    - Nodo I se recupera con prob γ
//
//  Parámetros:
//    β (beta)  : tasa de contagio por contacto [0, 1]
//    γ (gamma) : tasa de recuperación diaria   [0, 1]
//
//  Nota sobre R₀:
//    En una red homogénea R₀ ≈ β/γ, pero sobre un grafo real
//    el R₀ efectivo depende también de la estructura de la red
//    (grado promedio, clustering, topología). El valor β/γ
//    es solo una cota de referencia, no el R₀ real de la simulación.
// ─────────────────────────────────────────────────────────────
class SimuladorSIR {
public:
    SimuladorSIR(double beta, double gamma, unsigned seed = 42);

    /// Ejecuta la simulación durante `pasos` ticks sobre el grafo G.
    /// Devuelve la serie temporal de conteos SIR.
    std::vector<PuntoSIR> simular(Grafo& G, int pasos);

    double beta()  const { return beta_; }
    double gamma() const { return gamma_; }

    /// R₀ de referencia (válido en red homogénea, ver nota arriba).
    double R0() const { return beta_ / gamma_; }

private:
    double beta_;
    double gamma_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;

    bool ocurre(double prob) { return dist_(rng_) < prob; }
};

// ─────────────────────────────────────────────────────────────
//  Estrategias de contención
//  Cada función recibe el grafo original y devuelve una copia
//  modificada según la estrategia aplicada.
// ─────────────────────────────────────────────────────────────
namespace Estrategias {

    /// Sin intervención — devuelve copia del grafo original.
    Grafo ninguna(const Grafo& G);

    /// Aísla (remueve aristas de) los hubs más conectados.
    /// porcentaje: fracción de nodos a aislar [0, 1].
    Grafo aislarHubs(const Grafo& G, double porcentaje = 0.05);

    /// Rastreo de contactos por BFS: aísla vecinos de infectados.
    /// (Corregido: era "rastreaContacnosBFS")
    Grafo rastrearContactosBFS(const Grafo& G);

    /// Vacunación dirigida: marca como R los nodos de mayor centralidad.
    /// porcentaje: fracción de nodos a vacunar [0, 1].
    Grafo vacunacionDirigida(const Grafo& G, double porcentaje = 0.10);

    /// Devuelve el nombre legible de una estrategia por su clave.
    std::string nombre(const std::string& clave);
}
