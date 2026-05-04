#include "dijkstra.h"
#include <queue>
#include <cmath>
#include <limits>
#include <fstream>
#include <iostream>

namespace Dijkstra {

// ─────────────────────────────────────────────────────────────
//  caminoMasProbable
//  Transforma peso p → costo -log(p) para usar Dijkstra
//  (minimizar costo = maximizar probabilidad acumulada)
// ─────────────────────────────────────────────────────────────
Resultado caminoMasProbable(const Grafo& G, int origen, int destino) {
    Resultado res;
    res.encontrado = false;
    res.costo = std::numeric_limits<double>::infinity();

    if (!G.tieneNodo(origen) || !G.tieneNodo(destino))
        return res;

    // dist[v] = costo acumulado mínimo desde origen hasta v
    std::unordered_map<int, double> dist;
    std::unordered_map<int, int>    prev;

    for (int id : G.todosLosNodos()) {
        dist[id] = std::numeric_limits<double>::infinity();
        prev[id] = -1;
    }
    dist[origen] = 0.0;

    // Min-heap: (costo, nodo)
    using Par = std::pair<double, int>;
    std::priority_queue<Par, std::vector<Par>, std::greater<Par>> pq;
    pq.push({0.0, origen});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();

        if (d > dist[u]) continue;          // entrada obsoleta
        if (u == destino) break;            // llegamos al destino

        for (const Arista& a : G.vecinos(u)) {
            // Transformación: costo = -log(peso)
            // peso ∈ (0, 1] → costo ≥ 0
            double costo_arista = (a.peso > 0)
                ? -std::log(a.peso)
                : std::numeric_limits<double>::infinity();

            double nuevo_dist = dist[u] + costo_arista;
            if (nuevo_dist < dist[a.destino]) {
                dist[a.destino] = nuevo_dist;
                prev[a.destino] = u;
                pq.push({nuevo_dist, a.destino});
            }
        }
    }

    // Reconstruir camino
    if (dist[destino] == std::numeric_limits<double>::infinity())
        return res;   // no hay camino

    res.encontrado = true;
    res.costo = dist[destino];

    int actual = destino;
    while (actual != -1) {
        res.camino.push_back(actual);
        actual = prev[actual];
    }
    std::reverse(res.camino.begin(), res.camino.end());

    return res;
}

// ─────────────────────────────────────────────────────────────
//  distanciasDesde: BFS en saltos (sin pesos)
// ─────────────────────────────────────────────────────────────
std::unordered_map<int, int> distanciasDesde(const Grafo& G, int origen) {
    std::unordered_map<int, int> dist;

    if (!G.tieneNodo(origen)) return dist;

    std::queue<int> cola;
    dist[origen] = 0;
    cola.push(origen);

    while (!cola.empty()) {
        int u = cola.front(); cola.pop();
        for (const Arista& a : G.vecinos(u)) {
            if (dist.find(a.destino) == dist.end()) {
                dist[a.destino] = dist[u] + 1;
                cola.push(a.destino);
            }
        }
    }
    return dist;
}

// ─────────────────────────────────────────────────────────────
//  exportarCamino
// ─────────────────────────────────────────────────────────────
void exportarCamino(const Resultado& res,
                    const Grafo& G,
                    const std::string& path) {
    std::ofstream f(path);
    f << "paso,nodo_id,depto,ciudad,fecha,estado\n";

    if (!res.encontrado) {
        std::cout << "  [Dijkstra] No se encontro camino.\n";
        return;
    }

    for (int i = 0; i < (int)res.camino.size(); ++i) {
        int id = res.camino[i];
        const Nodo& n = G.getNodo(id);
        f << i << "," << id << ","
          << n.meta.depto << "," << n.meta.ciudad << ","
          << n.meta.fecha << "," << estadoStr(n.estado) << "\n";
    }

    std::cout << "  [Dijkstra] Camino exportado: " << path
              << " (" << res.camino.size() << " nodos, costo="
              << res.costo << ")\n";
}

} // namespace Dijkstra
