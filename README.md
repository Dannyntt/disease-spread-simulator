# 🦠 Disease Spread Simulator on Graphs

> Project — Data Structures and Algorithms  
> Modeling and simulation of epidemics using real COVID-19 Colombia data and graph theory

---

## 📌 Description

This project simulates how COVID-19 spread in Colombia by modeling real confirmed cases as a **contagion graph**, where each node is a confirmed case and each edge represents an epidemiological link (same department, close dates). The **SIR model** (Susceptible → Infected → Recovered) is implemented and four containment strategies are compared. Additionally, behavior is compared across three synthetic network topologies.

---

## 👥 Team

| Member | Responsibility |
|---|---|
| Maria Camila Rodriguez Gonzalez | Graph structure + BFS/DFS + Dijkstra |
| Daniela Giraldo Salas | Containment strategies + SIR model + general GitHub structure |
| Melissa Zuluaga Quintero | tree search implementation + report + base code |

---

## 🎯 Objectives

- Model the COVID-19 Colombia contagion network as a weighted undirected graph
- Implement step-by-step SIR simulation over the real network
- Identify super-spreaders using degree centrality and betweenness
- Find the most probable contagion chain between two cases (Dijkstra)
- Compare four containment strategies and measure their effectiveness
- Analyze how network topology affects the speed of propagation

---

## 🧠 Implemented Algorithms

| Algorithm | File | Use |
|---|---|---|
| BFS | `graph.cpp` | Contact tracing, distances |
| DFS | `graph.cpp` | Connected components |
| Dijkstra | `dijkstra.cpp` | Most probable contagion chain |
| Degree centrality | `graph.cpp` | Identify hubs |
| Betweenness centrality | `graph.cpp` | Critical bridge nodes (Brandes 2001) |
| SIR model | `model.cpp` | Spread simulation |
| Erdős–Rényi | `network_gen.cpp` | Random synthetic network |
| Watts–Strogatz | `network_gen.cpp` | Small-World network |
| Barabási–Albert | `network_gen.cpp` | Scale-Free network |

---

## 📁 Project Structure

```
disease-spread-simulator/
│
├── src/                        ← source code
│   ├── graph.h / graph.cpp     ← graph + BFS/DFS/centralities
│   ├── model.h / model.cpp     ← SIR simulation + strategies
│   ├── dijkstra.h / dijkstra.cpp
│   ├── network_gen.h / network_gen.cpp
│   ├── data_upd.h / data_upd.cpp ← CSV data loading
│   └── main.cpp
│
├── data/                       ← input datasets
│   ├── Casos1.csv              ← individual COVID-19 cases in Colombia
│   ├── covid-19-colombia-all.csv
│   ├── covid-19-colombia-deaths.csv
│   └── colombia_departamentos.csv
│
├── resultados/                 ← automatically generated outputs
│   ├── grafo_nodos.csv
│   ├── grafo_aristas.csv
│   ├── grafo_centralidades.csv
│   ├── grafo_componentes.csv
│   ├── cadena_contagio.csv
│   └── sir_*.csv               ← SIR curves per strategy
│
├── simulador.exe               ← precompiled executable (Windows)
├── CMakeLists.txt
├── .gitignore
└── README.md
```

---

## ⚙️ Build and Run

### Requirements
- C++17 or higher compiler (`g++` / `clang++`)
- CMake 3.15+ *(optional)*

### Option A — Build with CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
./simulador ../data/ ../resultados/
```

### Option B — Manual build with g++
```bash
g++ -std=c++17 -O2 -Isrc \
    src/main.cpp src/graph.cpp src/model.cpp \
    src/dijkstra.cpp src/network_gen.cpp src/data_upd.cpp \
    -o simulador

./simulador data/ resultados/
```

### Option C — Precompiled executable (Windows)
```bash
simulador.exe data/ resultados/
```

---

## 🚀 Program Output

```
[1/5] Loading data...
[2/5] Analyzing real graph...
[3/5] Most probable contagion chain (Dijkstra)...
[4/5] Simulating containment strategies...
[5/5] Comparing with synthetic networks...

COMPARATIVE SUMMARY OF STRATEGIES
Strategy                Peak I    Peak day  Total R   Reduction
No intervention         ...       ...       ...       0.0%
Isolate hubs (5%)       ...       ...       ...       XX.X%
BFS contact tracing     ...       ...       ...       XX.X%
Targeted vaccination    ...       ...       ...       XX.X%
```

All results are saved in `resultados/`.

---

## 📊 Data Used

Data from the **Instituto Nacional de Salud (INS) de Colombia**, publicly available.

- `Casos1.csv` — individual cases with date, department, city, age, sex
- `covid-19-colombia-all.csv` — national daily totals to calibrate β and γ

---

## 📚 References

- Newman, M. (2010). *Networks: An Introduction*. Oxford University Press.
- Kermack & McKendrick (1927). Contribution to the mathematical theory of epidemics.
- Brandes, U. (2001). A faster algorithm for betweenness centrality.
- Barabási & Albert (1999). Emergence of scaling in random networks.
- Watts & Strogatz (1998). Collective dynamics of small-world networks.
- AI assistance was used to help optimize parts of the code

---

## 📝 Academic note

Academic project — Data Structures and Algorithms course. Implemented in **C++17**.
