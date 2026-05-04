#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <chrono>
#include <filesystem>   // C++17 — reemplaza std::system para mkdir

#include "graph.h"
#include "model.h"
#include "data_upd.h"
#include "dijkstra.h"
#include "network_gen.h"

namespace CargaDatos {
    Grafo          cargarGrafoMicro   (const std::string& pathCasos);
    ParametrosSIR  calibrarParametros (const std::string& pathNacional);
}

// ─────────────────────────────────────────────────────────────
//  Utilidades de consola
// ─────────────────────────────────────────────────────────────
void imprimirSeparador(char c = '=', int n = 60) {
    std::cout << std::string(n, c) << "\n";
}

void imprimirMetricas(const Grafo& G, const std::string& nombre) {
    imprimirSeparador('-');
    std::cout << " Metricas: " << nombre << "\n";
    imprimirSeparador('-');

    auto grados = G.grados();
    int maxGrado = 0, minGrado = INT_MAX, totalGrado = 0;
    for (auto& [_, d] : grados) {
        maxGrado  = std::max(maxGrado, d);
        minGrado  = std::min(minGrado, d);
        totalGrado += d;
    }
    double promGrado = G.numNodos() > 0
        ? (double)totalGrado / G.numNodos() : 0;

    auto comps = G.componentesConexas();
    size_t maxComp = 0;
    for (auto& c : comps) maxComp = std::max(maxComp, c.size());

    double densidad = 0;
    if (G.numNodos() > 1) {
        long long maxAristas = (long long)G.numNodos() * (G.numNodos() - 1) / 2;
        densidad = (double)G.numAristas() / maxAristas;
    }

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  Nodos         : " << G.numNodos()   << "\n";
    std::cout << "  Aristas       : " << G.numAristas() << "\n";
    std::cout << "  Densidad      : " << densidad       << "\n";
    std::cout << "  Grado promedio: " << promGrado      << "\n";
    std::cout << "  Grado maximo  : " << maxGrado       << "\n";
    std::cout << "  Grado minimo  : " << minGrado       << "\n";
    std::cout << "  Componentes   : " << comps.size()   << "\n";
    std::cout << "  Comp. gigante : " << maxComp << " nodos\n";

    // Top 5 por centralidad de grado
    auto degCent = G.centralidadGrado();
    auto top5 = G.topKNodos(5, [&](int id) {
        return degCent.count(id) ? degCent.at(id) : 0.0;
    });
    std::cout << "  Top 5 hubs    : ";
    for (int id : top5) std::cout << id << " ";
    std::cout << "\n";

    // Top 5 por betweenness centrality
    auto bc = G.betweennessCentrality();
    auto top5bc = G.topKNodos(5, [&](int id) {
        return bc.count(id) ? bc.at(id) : 0.0;
    });
    std::cout << "  Top 5 betw.   : ";
    for (int id : top5bc) std::cout << id << " ";
    std::cout << "\n";

    auto cnt = G.contarEstados();
    std::cout << "  Estado SIR ini: S=" << cnt.S
              << " I=" << cnt.I << " R=" << cnt.R << "\n";
    imprimirSeparador('-');
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────
//  Guardar historia SIR a CSV
// ─────────────────────────────────────────────────────────────
void guardarHistoria(const std::vector<PuntoSIR>& historia,
                     const std::string& estrategia,
                     const std::string& outDir) {
    std::string path = outDir + "/sir_" + estrategia + ".csv";
    std::ofstream f(path);
    f << "tick,S,I,R\n";
    for (auto& p : historia)
        f << p.tick << "," << p.S << "," << p.I << "," << p.R << "\n";
}

// ─────────────────────────────────────────────────────────────
//  Resumen comparativo de estrategias
// ─────────────────────────────────────────────────────────────
void imprimirResumen(const std::map<std::string,
                     std::vector<PuntoSIR>>& resultados) {
    imprimirSeparador();
    std::cout << " RESUMEN COMPARATIVO DE ESTRATEGIAS\n";
    imprimirSeparador();
    std::cout << std::left
              << std::setw(24) << "Estrategia"
              << std::setw(12) << "Pico I"
              << std::setw(12) << "Dia pico"
              << std::setw(14) << "Total R (fin)"
              << "Reduccion pico\n";
    imprimirSeparador('-');

    int picoBaseline = 0;
    if (resultados.count("ninguna"))
        for (auto& p : resultados.at("ninguna"))
            picoBaseline = std::max(picoBaseline, p.I);

    for (auto& [estrategia, historia] : resultados) {
        int pico = 0, diaPico = 0, totalR = 0;
        for (auto& p : historia) {
            if (p.I > pico) { pico = p.I; diaPico = p.tick; }
            totalR = p.R;
        }
        double reduccion = picoBaseline > 0
            ? 100.0 * (picoBaseline - pico) / picoBaseline : 0;

        std::cout << std::left
                  << std::setw(24) << estrategia
                  << std::setw(12) << pico
                  << std::setw(12) << diaPico
                  << std::setw(14) << totalR
                  << std::fixed << std::setprecision(1)
                  << reduccion << "%\n";
    }
    imprimirSeparador();
}

// ─────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {

    std::string dataDir = (argc > 1) ? argv[1] : "./data";
    std::string outDir  = (argc > 2) ? argv[2] : "./resultados";

    // C++17: reemplaza std::system("mkdir -p ...") — portable en todos los SO
    std::filesystem::create_directories(outDir);

    imprimirSeparador();
    std::cout << " SIMULADOR SIR — COVID-19 COLOMBIA\n";
    std::cout << " Estructuras de Datos y Algoritmos\n";
    imprimirSeparador();
    std::cout << "\n";

    auto t0 = std::chrono::high_resolution_clock::now();

    // ── 1. Cargar datos ───────────────────────────────────────
    std::cout << "[1/5] Cargando datos...\n";
    Grafo G = CargaDatos::cargarGrafoMicro(dataDir + "/Casos1.csv");

    // CORRECCIÓN: el archivo se llama covid-19-colombia-all.csv
    auto params = CargaDatos::calibrarParametros(
        dataDir + "/covid-19-colombia-all.csv");

    std::cout << "  beta=" << params.beta
              << "  gamma=" << params.gamma
              << "  R0_ref=" << std::fixed << std::setprecision(2)
              << (params.beta / params.gamma) << "\n\n";

    // ── 2. Análisis del grafo ─────────────────────────────────
    std::cout << "[2/5] Analizando grafo real...\n";
    imprimirMetricas(G, "Grafo de Contagio COVID-19 Colombia");

    G.exportarNodos        (outDir + "/grafo_nodos.csv");
    G.exportarAristas      (outDir + "/grafo_aristas.csv");
    G.exportarGrados       (outDir + "/grafo_grados.csv");
    G.exportarCentralidades(outDir + "/grafo_centralidades.csv");
    G.exportarComponentes  (outDir + "/grafo_componentes.csv");
    std::cout << "  [OK] Grafo exportado a " << outDir << "/\n\n";

    // ── 3. Dijkstra: cadena de contagio más probable ──────────
    std::cout << "[3/5] Cadena de contagio mas probable (Dijkstra)...\n";
    {
        // Tomar el primer infectado y el nodo más lejano alcanzable como ejemplo
        auto nodos = G.todosLosNodos();
        int origen = -1, destino = -1;

        for (int id : nodos)
            if (G.getNodo(id).estado == EstadoSIR::I) { origen = id; break; }

        if (origen != -1) {
            auto distancias = Dijkstra::distanciasDesde(G, origen);
            int maxDist = 0;
            for (auto& [id, d] : distancias)
                if (d > maxDist) { maxDist = d; destino = id; }

            std::cout << "  Origen: " << origen
                      << " -> Destino: " << destino
                      << " (distancia=" << maxDist << " saltos)\n";

            auto res = Dijkstra::caminoMasProbable(G, origen, destino);
            Dijkstra::exportarCamino(res, G, outDir + "/cadena_contagio.csv");
        } else {
            std::cout << "  No hay nodos infectados en el grafo inicial.\n";
        }
    }
    std::cout << "\n";

    // ── 4. Simulación de estrategias ──────────────────────────
    std::cout << "[4/5] Simulando estrategias de contencion...\n";
    SimuladorSIR sim(params.beta, params.gamma, /*seed=*/42);
    const int PASOS = 60;

    struct EstrategiaFn {
        std::string clave;
        std::function<Grafo(const Grafo&)> fn;
    };

    std::vector<EstrategiaFn> estrategias = {
        {"ninguna",      [](const Grafo& g){ return Estrategias::ninguna(g); }},
        {"hubs",         [](const Grafo& g){ return Estrategias::aislarHubs(g, 0.05); }},
        {"rastreo_bfs",  [](const Grafo& g){ return Estrategias::rastrearContactosBFS(g); }},
        {"vacunacion",   [](const Grafo& g){ return Estrategias::vacunacionDirigida(g, 0.10); }},
    };

    std::map<std::string, std::vector<PuntoSIR>> resultados;

    for (auto& e : estrategias) {
        std::cout << "\n  Estrategia: " << Estrategias::nombre(e.clave) << "\n";
        Grafo G_mod    = e.fn(G);
        auto  historia = sim.simular(G_mod, PASOS);
        guardarHistoria(historia, e.clave, outDir);
        resultados[e.clave] = historia;
    }

    // ── 5. Comparación con redes sintéticas ───────────────────
    std::cout << "\n[5/5] Comparando con redes sinteticas...\n";
    {
        int n = std::min(G.numNodos(), 500); // usar tamaño similar al real
        int infectados_ini = std::max(1, n / 20);

        struct RedSint { std::string nombre; Grafo g; };
        std::vector<RedSint> redes = {
            {"Aleatoria (ER)",    NetworkGen::erdosRenyi(n, 0.02)},
            {"Small-World (WS)",  NetworkGen::wattsStrogatz(n, 4, 0.1)},
            {"Scale-Free (BA)",   NetworkGen::barabasiAlbert(n, 2)},
        };

        std::cout << "\n  Estadisticas de redes sinteticas:\n";
        for (auto& r : redes) {
            NetworkGen::infectarAleatorio(r.g, infectados_ini);
            NetworkGen::imprimirStats(r.g, r.nombre);

            auto historia = sim.simular(r.g, PASOS);
            guardarHistoria(historia, r.nombre, outDir);
            resultados["sint_" + r.nombre] = historia;
        }
    }

    // ── Resumen final ─────────────────────────────────────────
    std::cout << "\n";
    imprimirResumen(resultados);

    auto t1   = std::chrono::high_resolution_clock::now();
    double ss = std::chrono::duration<double>(t1 - t0).count();

    std::cout << "\n  Tiempo total : " << std::fixed
              << std::setprecision(2) << ss << " s\n";
    std::cout << "  Resultados en: " << outDir << "/\n";
    imprimirSeparador();

    return 0;
}
