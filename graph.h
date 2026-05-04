#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <climits>
#include <functional>

// ─────────────────────────────────────────────────────────────
//  Estado epidemiológico SIR
// ─────────────────────────────────────────────────────────────
enum class EstadoSIR { S, I, R };

inline std::string estadoStr(EstadoSIR e) {
    switch (e) {
        case EstadoSIR::S: return "S";
        case EstadoSIR::I: return "I";
        case EstadoSIR::R: return "R";
    }
    return "?";
}

// ─────────────────────────────────────────────────────────────
//  MetadatosCaso: datos del dominio COVID separados del grafo
//  No mezclar con la estructura del nodo para mantener
//  la independencia entre modelo de grafo y datos de entrada.
// ─────────────────────────────────────────────────────────────
struct MetadatosCaso {
    std::string tipo;   // Importado / Relacionado / En estudio
    std::string depto;
    std::string ciudad;
    int         edad  = 0;
    std::string sexo;
    std::string fecha;
};

// ─────────────────────────────────────────────────────────────
//  Nodo: unidad básica del grafo
//  Solo contiene información estructural + estado SIR.
//  Los metadatos del caso van en MetadatosCaso.
// ─────────────────────────────────────────────────────────────
struct Nodo {
    int        id;
    EstadoSIR  estado;
    MetadatosCaso meta;   // datos del dominio, separados

    Nodo() : id(0), estado(EstadoSIR::S) {}
};

// ─────────────────────────────────────────────────────────────
//  Arista: conexión entre dos nodos con peso temporal
//  peso     = 1.0 / (1 + dias_diferencia) — más reciente → más peso
//  diasDiff = diferencia en días entre los casos conectados
// ─────────────────────────────────────────────────────────────
struct Arista {
    int    destino;
    double peso;
    int    diasDiff;
};

// ─────────────────────────────────────────────────────────────
//  Grafo: grafo no dirigido con lista de adyacencia
//  Internamente usa unordered_map para acceso O(1) promedio.
// ─────────────────────────────────────────────────────────────
class Grafo {
public:

    // ── Construcción ─────────────────────────────────────────

    /// Agrega un nodo al grafo. Si ya existe, lo sobreescribe.
    void addNodo(const Nodo& n);

    /// Agrega una arista no dirigida entre u y v con el peso dado.
    /// No hace nada si alguno de los nodos no existe.
    void addArista(int u, int v, double peso, int diasDiff = 0);

    // ── Acceso ───────────────────────────────────────────────

    /// Devuelve referencia al nodo con el id dado (lanza si no existe).
    Nodo&       getNodo(int id)       { return nodos_.at(id); }
    const Nodo& getNodo(int id) const { return nodos_.at(id); }

    /// Verifica si el nodo existe en el grafo.
    bool tieneNodo(int id) const { return nodos_.count(id) > 0; }

    /// Verifica si existe una arista entre u y v.
    bool tieneArista(int u, int v) const;

    /// Devuelve la lista de aristas del nodo dado.
    const std::vector<Arista>& vecinos(int id) const;

    /// Devuelve todos los IDs de nodos en el grafo.
    std::vector<int> todosLosNodos() const;

    int numNodos()   const { return (int)nodos_.size(); }
    int numAristas() const { return totalAristas_; }

    // ── Conteos SIR ──────────────────────────────────────────

    struct ConteoSIR { int S = 0, I = 0, R = 0; };

    /// Cuenta cuántos nodos están en cada estado SIR.
    ConteoSIR contarEstados() const;

    // ── Algoritmos de grafos ──────────────────────────────────

    /// BFS desde múltiples fuentes hasta `saltos` pasos.
    /// Devuelve todos los nodos alcanzables dentro del rango.
    std::vector<int> bfs(const std::vector<int>& fuentes, int saltos) const;

    /// Devuelve el grado (número de vecinos) de cada nodo.
    std::unordered_map<int, int> grados() const;

    /// Centralidad de grado normalizada: grado / (n-1).
    std::unordered_map<int, double> centralidadGrado() const;

    /// Betweenness centrality (Brandes 2001).
    /// Mide qué tan seguido un nodo aparece en caminos más cortos entre otros.
    /// Complejidad: O(V * (V + E)).
    std::unordered_map<int, double> betweennessCentrality() const;

    /// Devuelve los k nodos con mayor puntuación según la función dada.
    std::vector<int> topKNodos(int k,
                               std::function<double(int)> puntuacion) const;

    /// Devuelve todas las componentes conexas del grafo (DFS iterativo).
    std::vector<std::vector<int>> componentesConexas() const;

    /// Elimina el nodo y todas sus aristas del grafo.
    void removerNodo(int id);

    /// Elimina solo las aristas del nodo (el nodo queda aislado).
    void removerAristas(int id);

    /// Devuelve una copia profunda del grafo.
    Grafo copia() const;

    // ── Exportación ──────────────────────────────────────────

    void exportarNodos       (const std::string& path) const;
    void exportarAristas     (const std::string& path) const;
    void exportarGrados      (const std::string& path) const;
    void exportarCentralidades(const std::string& path) const;
    void exportarComponentes  (const std::string& path) const;

private:
    std::unordered_map<int, Nodo>              nodos_;
    std::unordered_map<int, std::vector<Arista>> adyacencia_;
    int totalAristas_ = 0;

    static const std::vector<Arista> EMPTY_VEC;
};
