#pragma once

#include "graph.h"
#include <unordered_map>
#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────
//  Dijkstra sobre el grafo de contagio
//
//  Los pesos de las aristas son 1.0 / (1 + dias_diferencia),
//  así que un peso ALTO = contacto más reciente = más probable.
//
//  Para encontrar el camino de contagio más PROBABLE usamos
//  pesos negativos del logaritmo (transformación log-prob),
//  convirtiendo el producto de probabilidades en una suma
//  que Dijkstra puede minimizar.
//
//  camino_mas_probable(G, origen, destino):
//    Devuelve la secuencia de IDs del camino más probable
//    de contagio entre origen y destino.
//    Si no existe camino, devuelve vector vacío.
//
//  distancias_desde(G, origen):
//    Devuelve la distancia mínima (en saltos) desde origen
//    a todos los nodos alcanzables.
// ─────────────────────────────────────────────────────────────
namespace Dijkstra {

    struct Resultado {
        std::vector<int>          camino;      // IDs en orden origen→destino
        double                    costo;       // costo total del camino
        bool                      encontrado;  // false si no hay camino
    };

    /// Camino de contagio más probable entre origen y destino.
    /// Usa pesos -log(peso_arista) para maximizar probabilidad.
    Resultado caminoMasProbable(const Grafo& G, int origen, int destino);

    /// Distancia mínima en saltos desde origen a todos los nodos.
    /// Útil para calcular el radio de propagación desde un infectado.
    std::unordered_map<int, int> distanciasDesde(const Grafo& G, int origen);

    /// Exporta el camino encontrado a un archivo CSV.
    void exportarCamino(const Resultado& res,
                        const Grafo& G,
                        const std::string& path);
}
