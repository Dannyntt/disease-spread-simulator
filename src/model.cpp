//model SIRV:

//parametro beta y alpha, modelar la pandemia prara colombia??

#include "model.h"
#include <iostream>

//  se llama a la clase que fue generada en el header: SimuladorSIR
SimuladorSIR::SimuladorSIR(double beta, double gamma, unsigned seed)
    : beta_(beta), gamma_(gamma),
      rng_(seed), dist_(0.0, 1.0) {}

std::vector<PuntoSIR> SimuladorSIR::simular(Grafo& G, int pasos) {
    std::vector<PuntoSIR> historia;
    historia.reserve(pasos);

    for (int t = 0; t < pasos; ++t) {
        std::vector<int> aNuevoI;
        std::vector<int> aNuevoR;

        
        for (int u : G.todosLosNodos()) {
            Nodo& nodo = G.getNodo(u);
            if (nodo.estado != EstadoSIR::I) continue;

            // intentar infectar cada vecino susceptible
            for (auto& arista : G.vecinos(u)) {
                int v = arista.destino;
                if (G.getNodo(v).estado == EstadoSIR::S) {
                    double prob = beta_ * arista.peso;
                    if (ocurre(prob))
                        aNuevoI.push_back(v);
                }
            }

            // intenta recuperarse
            if (ocurre(gamma_))
                aNuevoR.push_back(u);
        }
        for (int v : aNuevoI) G.getNodo(v).estado = EstadoSIR::I;
        for (int v : aNuevoR) G.getNodo(v).estado = EstadoSIR::R;

        auto cnt = G.contarEstados();
        historia.push_back({t, cnt.S, cnt.I, cnt.R});
    }

    return historia;
}


namespace Estrategias {

Grafo ninguna(const Grafo& G) {
    return G.copia();
}

//  Aislar Hubs 
// Centralidad de grado: C_D(v) = deg(v) / (n-1)
// Aislamos el top-p% (mayor grado) removiendo sus nodos.
Grafo aislarHubs(const Grafo& G, double porcentaje) {
    Grafo g = G.copia();
    auto grados = g.grados();
    int k = std::max(1, (int)(g.numNodos() * porcentaje));

    auto hubs = g.topKNodos(k, [&](int id) {
        return (double)grados.count(id) ? grados.at(id) : 0;
    });

    std::cout << "  [hubs] " << hubs.size()
              << " nodos aislados (top "
              << (int)(porcentaje * 100) << "% por grado)\n";

    for (int h : hubs) g.removerNodo(h);
    return g;
}

// Rastreo BFS
// BFS 1-hop desde los nodos infectados.
// Todos los vecinos encontrados quedan en cuarentena
// (se remueven sus aristas para cortar la cadena de contagio).
Grafo rastrearContactosBFS(const Grafo& G) {
    Grafo g = G.copia();

    // Semillas: todos los infectados iniciales
    std::vector<int> infectados;
    for (int id : g.todosLosNodos())
        if (g.getNodo(id).estado == EstadoSIR::I)
            infectados.push_back(id);

    auto contactos = g.bfs(infectados, 1);

    //(cuarentena)
    int removidos = 0;
    for (int c : contactos) {
        if (g.getNodo(c).estado == EstadoSIR::S) {
            g.removerAristas(c);
            ++removidos;
        }
    }

    std::cout << "  [rastreo_bfs] " << removidos
              << " contactos en cuarentena (aristas removidas)\n";
    return g;
}

// Vacunación Dirigida:
// Betweenness centrality: mide cuántos caminos más cortos
// pasan por un nodo → nodos "puente" críticos para la difusión.
// Vacunamos el top-p% (estado a R, no pueden infectarse)
Grafo vacunacionDirigida(const Grafo& G, double porcentaje) {
    Grafo g = G.copia();
    auto bc = g.betweennessCentrality();
    int k = std::max(1, (int)(g.numNodos() * porcentaje));

    auto candidatos = g.topKNodos(k, [&](int id) {
        return bc.count(id) ? bc.at(id) : 0.0;
    });

    int vacunados = 0;
    for (int id : candidatos) {
        if (g.getNodo(id).estado == EstadoSIR::S) {
            g.getNodo(id).estado = EstadoSIR::R;
            ++vacunados;
        }
    }
    std::cout << "  [vacunacion] " << vacunados
              << " nodos vacunados (betweenness top "
              << (int)(porcentaje * 100) << "%)\n";
    return g;
}

std::string nombre(const std::string& clave) {
    if (clave == "ninguna")     return "Sin intervencion (baseline)";
    if (clave == "hubs")        return "Aislamiento de hubs (centralidad de grado)";
    if (clave == "rastreo_bfs") return "Rastreo de contactos (BFS 1-hop)";
    if (clave == "vacunacion")  return "Vacunacion dirigida (betweenness)";
    return clave;
}

} 
