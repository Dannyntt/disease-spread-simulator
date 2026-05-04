# 🦠 Simulador de Propagación de Enfermedades en Grafos

> Proyecto — Estructuras de Datos y Algoritmos  
> Modelado y simulación de epidemias sobre datos reales de COVID-19 Colombia usando teoría de grafos

---

## 📌 Descripción

Este proyecto simula cómo se propagó el COVID-19 en Colombia modelando los casos reales como un **grafo de contagio**, donde cada nodo es un caso confirmado y cada arista representa un vínculo epidemiológico (mismo departamento, fechas cercanas). Se implementa el modelo **SIR** (Susceptible → Infectado → Recuperado) y se comparan cuatro estrategias de contención. Adicionalmente se compara el comportamiento en tres topologías de red sintéticas.

---

## 👥 Equipo

| Integrante | Responsabilidad |
|---|---|
| Maria Camila Rodriguez Gonzalez| Estructura del grafo + BFS/DFS + Dijkstra |
| Daniela Giraldo Salas | Modelo SIR + estrategias de contención + estructura general del github|
| Karen Melissa Zuluaga Quintero| Redes sintéticas + centralidades + análisis + codigos base |

---

## 🎯 Objetivos

- Modelar la red de contagio de COVID-19 Colombia como grafo no dirigido ponderado
- Implementar simulación SIR paso a paso sobre la red real
- Identificar super-contagiadores con centralidad de grado y betweenness
- Encontrar la cadena de contagio más probable entre dos casos (Dijkstra)
- Comparar cuatro estrategias de contención y medir su efectividad
- Analizar cómo la topología de red afecta la velocidad de propagación

---

## 🧠 Algoritmos Implementados

| Algoritmo | Archivo | Uso |
|---|---|---|
| BFS | `graph.cpp` | Rastreo de contactos, distancias |
| DFS | `graph.cpp` | Componentes conexas |
| Dijkstra | `dijkstra.cpp` | Cadena de contagio más probable |
| Centralidad de grado | `graph.cpp` | Identificar hubs |
| Betweenness centrality | `graph.cpp` | Nodos puente críticos (Brandes 2001) |
| Modelo SIR | `model.cpp` | Simulación de propagación |
| Erdős–Rényi | `network_gen.cpp` | Red aleatoria sintética |
| Watts–Strogatz | `network_gen.cpp` | Red Small-World |
| Barabási–Albert | `network_gen.cpp` | Red Scale-Free |

---

## 📁 Estructura del Proyecto

```
disease-spread-simulator/
│
├── src/                        ← código fuente
│   ├── graph.h / graph.cpp     ← grafo + BFS/DFS/centralidades
│   ├── model.h / model.cpp     ← simulación SIR + estrategias
│   ├── dijkstra.h / dijkstra.cpp
│   ├── network_gen.h / network_gen.cpp
│   ├── data_upd.h / data_upd.cpp ← carga de datos CSV
│   └── main.cpp
│
├── data/                       ← datasets de entrada
│   ├── Casos1.csv              ← casos individuales COVID-19 Colombia
│   ├── covid-19-colombia-all.csv
│   ├── covid-19-colombia-deaths.csv
│   └── colombia_departamentos.csv
│
├── resultados/                 ← salidas generadas automáticamente
│   ├── grafo_nodos.csv
│   ├── grafo_aristas.csv
│   ├── grafo_centralidades.csv
│   ├── grafo_componentes.csv
│   ├── cadena_contagio.csv
│   └── sir_*.csv               ← curvas SIR por estrategia
│
├── simulador.exe               ← ejecutable precompilado (Windows)
├── CMakeLists.txt
├── .gitignore
└── README.md
```

---

## ⚙️ Compilación y Ejecución

### Requisitos
- Compilador C++17 o superior (`g++` / `clang++`)
- CMake 3.15+ *(opcional)*

### Opción A — Compilar con CMake
```bash
mkdir build && cd build
cmake ..
cmake --build .
./simulador ../data/ ../resultados/
```

### Opción B — Compilar manual con g++
```bash
g++ -std=c++17 -O2 -Isrc \
    src/main.cpp src/graph.cpp src/model.cpp \
    src/dijkstra.cpp src/network_gen.cpp src/data_upd.cpp \
    -o simulador

./simulador data/ resultados/
```

### Opción C — Ejecutable precompilado (Windows)
```bash
simulador.exe data/ resultados/
```

---

## 🚀 Salida del Programa

```
[1/5] Cargando datos...
[2/5] Analizando grafo real...
[3/5] Cadena de contagio mas probable (Dijkstra)...
[4/5] Simulando estrategias de contencion...
[5/5] Comparando con redes sinteticas...

RESUMEN COMPARATIVO DE ESTRATEGIAS
Estrategia              Pico I    Dia pico  Total R   Reduccion
Sin intervencion        ...       ...       ...       0.0%
Aislar hubs (5%)        ...       ...       ...       XX.X%
Rastreo BFS             ...       ...       ...       XX.X%
Vacunacion dirigida     ...       ...       ...       XX.X%
```

Todos los resultados se guardan en `resultados/`.

---

## 📊 Datos Utilizados

Datos del **Instituto Nacional de Salud (INS) de Colombia**, disponibles públicamente.

- `Casos1.csv` — casos individuales con fecha, departamento, ciudad, edad, sexo
- `covid-19-colombia-all.csv` — totales nacionales diarios para calibrar β y γ

---

## 📚 Referencias

- Newman, M. (2010). *Networks: An Introduction*. Oxford University Press.
- Kermack & McKendrick (1927). Contribution to the mathematical theory of epidemics.
- Brandes, U. (2001). A faster algorithm for betweenness centrality.
- Barabási & Albert (1999). Emergence of scaling in random networks.
- Watts & Strogatz (1998). Collective dynamics of small-world networks.
- se utilizo un poco de ayuda de IA para mejorar la optimizacion del codigo

---

## 📝 Nota académica

Proyecto académico — Curso de Estructuras de Datos y Algoritmos. Implementado en **C++17**.
