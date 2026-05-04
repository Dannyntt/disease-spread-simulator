#include "graph.h"
#include <fstream>
#include <iostream>
#include <numeric>
#include <stack>
#include "data_upd.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_map>
#include <climits>

const std::vector<Arista> Grafo::EMPTY_VEC = {};

// ── Construcción ──────────────────────────────────────────
void Grafo::addNodo(const Nodo& n) {
    nodos_[n.id] = n;
    if (adyacencia_.find(n.id) == adyacencia_.end())
        adyacencia_[n.id] = {};
}

void Grafo::addArista(int u, int v, double peso, int diasDiff) {
    if (!tieneNodo(u) || !tieneNodo(v)) return;
    adyacencia_[u].push_back({v, peso, diasDiff});
    adyacencia_[v].push_back({u, peso, diasDiff});
    ++totalAristas_;
}

bool Grafo::tieneArista(int u, int v) const {
    auto it = adyacencia_.find(u);
    if (it == adyacencia_.end()) return false;
    for (auto& a : it->second)
        if (a.destino == v) return true;
    return false;
}

const std::vector<Arista>& Grafo::vecinos(int id) const {
    auto it = adyacencia_.find(id);
    return it != adyacencia_.end() ? it->second : EMPTY_VEC;
}

std::vector<int> Grafo::todosLosNodos() const {
    std::vector<int> ids;
    ids.reserve(nodos_.size());
    for (auto& [id, _] : nodos_) ids.push_back(id);
    return ids;
}

// ── Conteos SIR ───────────────────────────────────────────
Grafo::ConteoSIR Grafo::contarEstados() const {
    ConteoSIR cnt;
    for (auto& [_, n] : nodos_) {
        if (n.estado == EstadoSIR::S) ++cnt.S;
        else if (n.estado == EstadoSIR::I) ++cnt.I;
        else ++cnt.R;
    }
    return cnt;
}

// ── BFS ───────────────────────────────────────────────────
std::vector<int> Grafo::bfs(const std::vector<int>& fuentes, int saltos) const {
    std::unordered_map<int, int> distancia;
    std::queue<int> cola;

    for (int s : fuentes) {
        if (tieneNodo(s)) {
            distancia[s] = 0;
            cola.push(s);
        }
    }

    std::vector<int> alcanzables;
    while (!cola.empty()) {
        int u = cola.front(); cola.pop();
        alcanzables.push_back(u);

        if (distancia[u] < saltos) {
            for (auto& a : vecinos(u)) {
                if (distancia.find(a.destino) == distancia.end()) {
                    distancia[a.destino] = distancia[u] + 1;
                    cola.push(a.destino);
                }
            }
        }
    }
    return alcanzables;
}

// ── Grados ────────────────────────────────────────────────
std::unordered_map<int, int> Grafo::grados() const {
    std::unordered_map<int, int> g;
    for (auto& [id, aristas] : adyacencia_)
        g[id] = (int)aristas.size();
    return g;
}

// ── Centralidad de grado ──────────────────────────────────
std::unordered_map<int, double> Grafo::centralidadGrado() const {
    auto g = grados();
    std::unordered_map<int, double> c;
    double n = (double)numNodos();
    for (auto& [id, deg] : g)
        c[id] = (n > 1) ? deg / (n - 1.0) : 0.0;
    return c;
}

// ── Betweenness Centrality (Brandes 2001) ─────────────────
// Para cada nodo s:
//   1. BFS para hallar caminos más cortos
//   2. Acumula contribuciones hacia atrás (back-propagation)
// Complejidad: O(V × (V + E))
std::unordered_map<int, double> Grafo::betweennessCentrality() const {
    std::unordered_map<int, double> bc;
    for (auto& [id, _] : nodos_) bc[id] = 0.0;

    for (auto& [s, _] : nodos_) {
        // Pila de nodos en orden de descubrimiento
        std::stack<int> pila;
        // Predecesores en el camino más corto
        std::unordered_map<int, std::vector<int>> pred;
        // Número de caminos más cortos desde s
        std::unordered_map<int, long long> sigma;
        // Distancia desde s
        std::unordered_map<int, int> dist;

        for (auto& [v, _2] : nodos_) {
            sigma[v] = 0;
            dist[v]  = -1;
        }
        sigma[s] = 1;
        dist[s]  = 0;

        std::queue<int> cola;
        cola.push(s);

        while (!cola.empty()) {
            int v = cola.front(); cola.pop();
            pila.push(v);

            for (auto& a : vecinos(v)) {
                int w = a.destino;
                if (dist[w] < 0) {   // primera visita
                    cola.push(w);
                    dist[w] = dist[v] + 1;
                }
                if (dist[w] == dist[v] + 1) {
                    sigma[w] += sigma[v];
                    pred[w].push_back(v);
                }
            }
        }

        // Acumulación
        std::unordered_map<int, double> delta;
        for (auto& [v, _2] : nodos_) delta[v] = 0.0;

        while (!pila.empty()) {
            int w = pila.top(); pila.pop();
            for (int v : pred[w]) {
                if (sigma[w] != 0)
                    delta[v] += ((double)sigma[v] / sigma[w]) * (1.0 + delta[w]);
            }
            if (w != s) bc[w] += delta[w];
        }
    }

    // Normalizar (grafo no dirigido: dividir entre 2)
    double factor = 2.0;
    for (auto& [id, val] : bc) val /= factor;

    return bc;
}

// ── Top-K nodos ───────────────────────────────────────────
std::vector<int> Grafo::topKNodos(int k,
    std::function<double(int)> puntuacion) const {

    std::vector<std::pair<double, int>> scores;
    for (auto& [id, _] : nodos_)
        scores.push_back({puntuacion(id), id});

    std::sort(scores.begin(), scores.end(),
              [](auto& a, auto& b){ return a.first > b.first; });

    std::vector<int> result;
    int lim = std::min(k, (int)scores.size());
    for (int i = 0; i < lim; ++i)
        result.push_back(scores[i].second);
    return result;
}

// ── Componentes conexas (DFS iterativo) ───────────────────
std::vector<std::vector<int>> Grafo::componentesConexas() const {
    std::unordered_set<int> visitados;
    std::vector<std::vector<int>> comps;

    for (auto& [id, _] : nodos_) {
        if (visitados.count(id)) continue;

        std::vector<int> comp;
        std::stack<int> pila;
        pila.push(id);

        while (!pila.empty()) {
            int u = pila.top(); pila.pop();
            if (visitados.count(u)) continue;
            visitados.insert(u);
            comp.push_back(u);
            for (auto& a : vecinos(u))
                if (!visitados.count(a.destino))
                    pila.push(a.destino);
        }
        comps.push_back(comp);
    }
    return comps;
}

// ── Modificación ──────────────────────────────────────────
void Grafo::removerNodo(int id) {
    // Eliminar aristas hacia este nodo desde sus vecinos
    for (auto& a : vecinos(id)) {
        auto& lista = adyacencia_[a.destino];
        lista.erase(
            std::remove_if(lista.begin(), lista.end(),
                [id](const Arista& x){ return x.destino == id; }),
            lista.end()
        );
        --totalAristas_;
    }
    adyacencia_.erase(id);
    nodos_.erase(id);
}

void Grafo::removerAristas(int id) {
    for (auto& a : vecinos(id)) {
        auto& lista = adyacencia_[a.destino];
        lista.erase(
            std::remove_if(lista.begin(), lista.end(),
                [id](const Arista& x){ return x.destino == id; }),
            lista.end()
        );
    }
    adyacencia_[id].clear();
    // Recalcular totalAristas_
    totalAristas_ = 0;
    for (auto& [_, v] : adyacencia_)
        totalAristas_ += (int)v.size();
    totalAristas_ /= 2;
}

// ── Copia profunda ────────────────────────────────────────
Grafo Grafo::copia() const {
    Grafo g;
    g.nodos_       = nodos_;
    g.adyacencia_  = adyacencia_;
    g.totalAristas_ = totalAristas_;
    return g;
}

// ── Exportar CSV ──────────────────────────────────────────
void Grafo::exportarNodos(const std::string& path) const {
    std::ofstream f(path);
    f << "id,estado,tipo,depto,ciudad,edad,sexo,fecha\n";
    for (auto& [id, n] : nodos_) {
        f << id << "," << estadoStr(n.estado) << ","
          << n.tipo << "," << n.depto << "," << n.ciudad << ","
          << n.edad << "," << n.sexo << "," << n.fecha << "\n";
    }
}

void Grafo::exportarAristas(const std::string& path) const {
    std::ofstream f(path);
    f << "origen,destino,peso,dias_diff\n";
    std::unordered_set<long long> vistos;
    for (auto& [u, aristas] : adyacencia_) {
        for (auto& a : aristas) {
            long long key = (long long)std::min(u, a.destino) * 100000
                          + std::max(u, a.destino);
            if (!vistos.count(key)) {
                vistos.insert(key);
                f << u << "," << a.destino << ","
                  << a.peso << "," << a.diasDiff << "\n";
            }
        }
    }
}

// ── Exportar distribución de grados ──────────────────────
void Grafo::exportarGrados(const std::string& path) const {
    std::ofstream f(path);
    f << "id,grado,tipo,depto\n";
    for (auto& [id, aristas] : adyacencia_) {
        auto& n = nodos_.at(id);
        f << id << "," << aristas.size() << ","
          << n.tipo << "," << n.depto << "\n";
    }
}

void Grafo::exportarCentralidades(const std::string& path) const {
    auto degC = centralidadGrado();
    auto bc   = betweennessCentrality();

    std::ofstream f(path);
    f << "id,centralidad_grado,betweenness,grado,tipo,depto\n";
    for (auto& [id, n] : nodos_) {
        int grado = (int)adyacencia_.at(id).size();
        f << id << ","
          << (degC.count(id) ? degC.at(id) : 0.0) << ","
          << (bc.count(id)   ? bc.at(id)   : 0.0) << ","
          << grado << ","
          << n.tipo << "," << n.depto << "\n";
    }
}


void Grafo::exportarComponentes(const std::string& path) const {
    auto comps = componentesConexas();
    std::ofstream f(path);
    f << "componente_id,nodo_id,tamano_componente\n";
    for (int ci = 0; ci < (int)comps.size(); ++ci) {
        int tam = (int)comps[ci].size();
        for (int nid : comps[ci])
            f << ci << "," << nid << "," << tam << "\n";
    }
}
