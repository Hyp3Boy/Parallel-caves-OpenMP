# ParallelCavesOMP

**ParallelCavesOMP** es un proyecto que genera mapas de cuevas procedurales utilizando el algoritmo de Marching Squares. La generación y el procesamiento del mapa están paralelizados utilizando **OpenMP** para aprovechar la arquitectura de memoria compartida de los sistemas multi-núcleo modernos. La visualización se realiza con SFML y el proceso de generación está animado.

## Características

*   Generación procedural de mapas de cuevas.
*   Suavizado del mapa mediante un autómata celular.
*   Creación de la malla de la cueva utilizando el algoritmo Marching Squares.
*   **Animación en tiempo real** que muestra el proceso de suavizado y la generación final de la malla.
*   Paralelización de la generación y procesamiento con **OpenMP**.
*   Visualización del resultado con **SFML 3**.
*   Sistema de compilación moderno y portable con **CMake**.

## Requisitos Previos

Antes de compilar y ejecutar, necesitarás tener instaladas las siguientes dependencias:

1.  **Un Compilador C++ Moderno que soporte C++17 y OpenMP:**
    *   **macOS:** Se recomienda **Clang** (instalado con las Herramientas de Línea de Comandos de Xcode).
    *   **Linux:** Se recomienda **GCC**.
    *   **Windows:** Se recomienda **MSVC** (Visual Studio 2019 o superior).

2.  **CMake:**
    *   Versión **3.14 o superior**.
    *   Descarga e instala desde [cmake.org](https://cmake.org/download/).

3.  **Git:**
    *   Necesario para que CMake descargue SFML automáticamente.

4.  **Soporte para OpenMP:**
    *   **macOS:** Se requiere la librería `libomp`. La forma más sencilla de instalarla es a través de Homebrew:
        ```bash
        brew install libomp
        ```
        *El script de CMake está configurado para encontrar esta instalación de Homebrew automáticamente.*
    *   **Linux:** Generalmente, el soporte para OpenMP (`libgomp`) viene incluido con el compilador GCC (instalado con paquetes como `build-essential`). No suele requerir una instalación por separado.
    *   **Windows (MSVC):** El soporte para OpenMP está integrado y se activa con un flag del compilador, del cual CMake se encarga.

5.  **SFML (Simple and Fast Multimedia Library):**
    *   **¡No necesitas instalarla manualmente!** El proyecto está configurado con `FetchContent` de CMake, que descargará y compilará automáticamente la versión 3.0.1 de SFML la primera vez que compiles.

## Compilación y Ejecución

El proyecto incluye un `Makefile` que simplifica los comandos de CMake.

1.  **Clona el repositorio:**
    ```bash
    git clone https://github.com/TU_USUARIO/ParallelCavesOMP.git
    cd ParallelCavesOMP
    ```
    *(Reemplaza `TU_USUARIO` con tu nombre de usuario).*

2.  **Compilación (Primera Vez):**
    La primera vez que compiles el proyecto, es una buena práctica usar el comando `build`. Esto creará un directorio `build/`, configurará el proyecto con CMake y lo compilará. Este paso puede tardar un poco la primera vez, ya que también descargará SFML.
    ```bash
    make build
    ```
    *En macOS, este comando se asegurará de usar Clang como compilador para evitar problemas de compatibilidad.*

3.  **Ejecución (Uso Diario):**
    Después de la primera compilación, o para compilar los cambios y ejecutar el programa en un solo paso, simplemente usa el comando `run`:
    ```bash
    make run
    ```
    Este comando recompilará únicamente los archivos que hayan cambiado y luego lanzará la aplicación.

4.  **Limpiar los Archivos de Compilación:**
    Si quieres eliminar todos los archivos generados y empezar de cero, usa el comando `clean`:
    ```bash
    make clean
    ```

## Estructura del Proyecto


ParallelCavesOMP/
├── CMakeLists.txt # Script de compilación principal para CMake
├── Makefile # Envoltorio para simplificar los comandos de CMake
├── README.md # Este archivo
└── src/ # Directorio del código fuente
├── main.cpp # Punto de entrada, animación, renderizado SFML
├── CaveGenerator.hpp # Definición de la clase para generar el mapa
├── CaveGenerator.cpp # Implementación de CaveGenerator
├── MarchingSquares.hpp # Definición de la clase para Marching Squares
└── MarchingSquares.cpp # Implementación de Marching Squares
