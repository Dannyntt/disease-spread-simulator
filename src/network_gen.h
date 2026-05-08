#pragma once

#include "graph.h"
#include <string>

// ─────────────────────────────────────────────────────────────
//  NetworkGen: generador de redes sintéticas para comparar
//  el comportamiento de la epidemia en distintas topologías
//
//  Topologías disponibles:
//

//  1. Red aleatoria (Erdős–Rényi G(n,p))
//     Cada par de nodos se conecta con probabilidad p.
//     Grado promedio ≈ p*(n-1). Sin estructura de comunidades.
//
//  2. Red Small-World (Watts–Strogatz)
//     Partimos de un anillo regular (cada nodo conectado a k vecinos)
//     y reconectamos cada arista con probabilidad p.
//     Alta clusterización + caminos cortos → modela redes sociales reales.
//
//  3. Red Scale-Free (Barabási–Albert)
//     Crecimiento con preferential attachment: los nodos más conectados
//     atraen más conexiones. Produce distribución de grado ley de potencia.
//     Modela internet, redes de citas, contactos en epidemias reales.
// ─────────────────────────────────────────────────────────────
namespace NetworkGen {

    /// Red aleatoria Erdős–Rényi G(n, p)
    /// n: número de nodos, p: probabilidad de arista [0,1]
    Grafo erdosRenyi(int n, double p, unsigned seed = 42);

    /// Red Small-World Watts–Strogatz
    /// n: nodos, k: vecinos iniciales por lado, p: prob. reconexión
    Grafo wattsStrogatz(int n, int k, double p, unsigned seed = 42);

    /// Red Scale-Free Barabási–Albert
    /// n: nodos totales, m: aristas nuevas por nodo añadido
    Grafo barabasiAlbert(int n, int m, unsigned seed = 42);

    /// Infección inicial: marca `num_infectados` nodos al azar como I
    void infectarAleatorio(Grafo& G, int num_infectados, unsigned seed = 99);

    /// Imprime estadísticas básicas de la red generada
    void imprimirStats(const Grafo& G, const std::string& nombre);
}
