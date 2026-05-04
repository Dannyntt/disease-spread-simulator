#include "network_gen.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>

namespace NetworkGen {

// ─────────────────────────────────────────────────────────────
//  Helper: crear nodo base para redes sintéticas
// ─────────────────────────────────────────────────────────────
static Nodo crearNodo(int id) {
    Nodo n;
    n.id     = id;
    n.estado = EstadoSIR::S;
    n.meta.tipo  = "Sintetico";
    n.meta.depto = "N/A";
    n.meta.ciudad= "N/A";
    n.meta.fecha = "N/A";
    return n;
}

// ─────────────────────────────────────────────────────────────
//  Erdős–Rényi G(n, p)
// ─────────────────────────────────────────────────────────────
Grafo erdosRenyi(int n, double p, unsigned seed) {
    Grafo G;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < n; ++i)
        G.addNodo(crearNodo(i));

    for (int u = 0; u < n; ++u)
        for (int v = u + 1; v < n; ++v)
            if (dist(rng) < p)
                G.addArista(u, v, 1.0);

    return G;
}

// ─────────────────────────────────────────────────────────────
//  Watts–Strogatz Small-World
// ─────────────────────────────────────────────────────────────
Grafo wattsStrogatz(int n, int k, double p, unsigned seed) {
    Grafo G;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    std::uniform_int_distribution<int>     randNodo(0, n - 1);

    for (int i = 0; i < n; ++i)
        G.addNodo(crearNodo(i));

    // Paso 1: anillo regular — cada nodo conectado a k vecinos por lado
    for (int u = 0; u < n; ++u)
        for (int j = 1; j <= k; ++j) {
            int v = (u + j) % n;
            if (!G.tieneArista(u, v))
                G.addArista(u, v, 1.0);
        }

    // Paso 2: reconexión aleatoria con probabilidad p
    for (int u = 0; u < n; ++u) {
        for (int j = 1; j <= k; ++j) {
            if (prob(rng) < p) {
                int v_old = (u + j) % n;
                int v_new;
                int intentos = 0;
                do {
                    v_new = randNodo(rng);
                    ++intentos;
                } while ((v_new == u || G.tieneArista(u, v_new)) && intentos < 100);

                if (intentos < 100 && v_new != u) {
                    // No podemos remover aristas individuales fácilmente,
                    // así que simplemente agregamos la nueva (el anillo base queda)
                    G.addArista(u, v_new, 1.0);
                }
            }
        }
    }

    return G;
}

// ─────────────────────────────────────────────────────────────
//  Barabási–Albert Scale-Free
//  Preferential attachment: P(conectar a v) ∝ grado(v)
// ─────────────────────────────────────────────────────────────
Grafo barabasiAlbert(int n, int m, unsigned seed) {
    Grafo G;
    std::mt19937 rng(seed);

    if (n <= m) m = n - 1;

    // Red inicial: clique de m+1 nodos
    for (int i = 0; i <= m; ++i)
        G.addNodo(crearNodo(i));
    for (int u = 0; u <= m; ++u)
        for (int v = u + 1; v <= m; ++v)
            G.addArista(u, v, 1.0);

    // Lista de extremos para muestreo proporcional al grado
    // Cada vez que se agrega una arista (u,v) se añaden u y v a la lista
    std::vector<int> extremos;
    for (int u = 0; u <= m; ++u)
        for (int v = u + 1; v <= m; ++v) {
            extremos.push_back(u);
            extremos.push_back(v);
        }

    // Añadir nodos uno por uno
    for (int nuevo = m + 1; nuevo < n; ++nuevo) {
        G.addNodo(crearNodo(nuevo));

        std::unordered_set<int> elegidos;
        int intentos = 0;

        while ((int)elegidos.size() < m && intentos < 10000) {
            std::uniform_int_distribution<int> pick(0, (int)extremos.size() - 1);
            int candidato = extremos[pick(rng)];
            if (candidato != nuevo && !elegidos.count(candidato))
                elegidos.insert(candidato);
            ++intentos;
        }

        for (int v : elegidos) {
            G.addArista(nuevo, v, 1.0);
            extremos.push_back(nuevo);
            extremos.push_back(v);
        }
    }

    return G;
}

// ─────────────────────────────────────────────────────────────
//  infectarAleatorio
// ─────────────────────────────────────────────────────────────
void infectarAleatorio(Grafo& G, int num_infectados, unsigned seed) {
    auto nodos = G.todosLosNodos();
    std::mt19937 rng(seed);
    std::shuffle(nodos.begin(), nodos.end(), rng);

    int infectados = 0;
    for (int id : nodos) {
        if (infectados >= num_infectados) break;
        G.getNodo(id).estado = EstadoSIR::I;
        ++infectados;
    }
}

// ─────────────────────────────────────────────────────────────
//  imprimirStats
// ─────────────────────────────────────────────────────────────
void imprimirStats(const Grafo& G, const std::string& nombre) {
    auto grados = G.grados();
    double suma = 0;
    int maxG = 0;
    for (auto& [_, d] : grados) {
        suma += d;
        maxG = std::max(maxG, d);
    }
    double promedio = G.numNodos() > 0 ? suma / G.numNodos() : 0;

    auto comps = G.componentesConexas();
    size_t maxComp = 0;
    for (auto& c : comps) maxComp = std::max(maxComp, c.size());

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  [" << nombre << "]\n";
    std::cout << "    Nodos        : " << G.numNodos()   << "\n";
    std::cout << "    Aristas      : " << G.numAristas() << "\n";
    std::cout << "    Grado prom.  : " << promedio       << "\n";
    std::cout << "    Grado max    : " << maxG           << "\n";
    std::cout << "    Componentes  : " << comps.size()   << "\n";
    std::cout << "    Comp. gigante: " << maxComp << " nodos\n\n";
}

} // namespace NetworkGen
