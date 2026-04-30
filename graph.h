#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <climits>
#include <functional>


enum class EstadoSIR { S, I, R };

inline std::string estadoStr(EstadoSIR e) {
    switch (e) {
        case EstadoSIR::S: return "S";
        case EstadoSIR::I: return "I";
        case EstadoSIR::R: return "R";
    }
    return "?";
}

struct Nodo {
    int         id;
    EstadoSIR   estado;
    std::string tipo;       // Importado / Relacionado / En estudio
    std::string depto;
    std::string ciudad;
    int         edad;
    std::string sexo;
    std::string fecha;

    Nodo() : id(0), estado(EstadoSIR::S), edad(0) {}
};

struct Arista {
    int    destino;
    double peso;       // 1.0 / (1 + dias_diferencia)
    int    diasDiff;
};


//  Grafo no dirigido con lista de adyacencia
//    nodos_     : unordered_map<id, Nodo>
//    adyacencia_: unordered_map<id, vector<Arista>>

class Grafo {
public:
    // ── Construcción ──────────────────────────────────────
    void addNodo(const Nodo& n);
    void addArista(int u, int v, double peso, int diasDiff = 0);

    // ── Acceso ────────────────────────────────────────────
    Nodo&              getNodo(int id)       { return nodos_.at(id); }
    const Nodo&        getNodo(int id) const { return nodos_.at(id); }
    bool               tieneNodo(int id) const { return nodos_.count(id) > 0; }
    bool               tieneArista(int u, int v) const;

    const std::vector<Arista>& vecinos(int id) const;

    std::vector<int> todosLosNodos() const;
    int numNodos()   const { return (int)nodos_.size(); }
    int numAristas() const { return totalAristas_; }

    // ── Conteos SIR ───────────────────────────────────────
    struct ConteoSIR { int S = 0, I = 0, R = 0; };
    ConteoSIR contarEstados() const;


    std::vector<int> bfs(const std::vector<int>& fuentes, int saltos) const;
    std::unordered_map<int, int> grados() const;
    std::unordered_map<int, double> centralidadGrado() const;
    std::unordered_map<int, double> betweennessCentrality() const;

    std::vector<int> topKNodos(int k,
        std::function<double(int)> puntuacion) const;
    std::vector<std::vector<int>> componentesConexas() const;

    void removerNodo(int id);
    void removerAristas(int id);   // quita solo las aristas del nodo

  
    Grafo copia() const;
    void exportarNodos(const std::string& path) const;
    void exportarAristas(const std::string& path) const;
    void exportarGrados(const std::string& path) const;
    void exportarCentralidades(const std::string& path) const;
    void exportarComponentes(const std::string& path) const;

private:
    std::unordered_map<int, Nodo>           nodos_;
    std::unordered_map<int, std::vector<Arista>> adyacencia_;
    int totalAristas_ = 0;

    static const std::vector<Arista> EMPTY_VEC;
};