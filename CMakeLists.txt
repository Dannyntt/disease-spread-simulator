cmake_minimum_required(VERSION 3.15)
project(SimuladorPropagacion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Directorios de cabeceras ───────────────────────────────────
include_directories(src)

# ── Fuentes ───────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/graph.cpp
    src/model.cpp
    src/dijkstra.cpp
    src/network_gen.cpp
    src/data_upd.cpp
)

# ── Ejecutable ────────────────────────────────────────────────
add_executable(simulador ${SOURCES})

# ── Flags de compilación ──────────────────────────────────────
target_compile_options(simulador PRIVATE
    -Wall
    -Wextra
    -O2
)

# ── Mensaje de ayuda al configurar ────────────────────────────
message(STATUS "")
message(STATUS "  Simulador de Propagacion de Enfermedades")
message(STATUS "  Compilar : cmake --build .")
message(STATUS "  Ejecutar : ./simulador data/ resultados/")
message(STATUS "")
