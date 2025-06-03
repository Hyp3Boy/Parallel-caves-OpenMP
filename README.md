# ParallelCavesMPI

**ParallelCavesMPI** es un proyecto que genera mapas de cuevas procedurales utilizando el algoritmo de Marching Squares. La generación y el procesamiento del mapa están paralelizados utilizando MPI (Message Passing Interface) para mejorar el rendimiento en sistemas multi-núcleo o clusters. La visualización se realiza con SFML.

## Características

*   Generación procedural de mapas de cuevas.
*   Suavizado del mapa mediante un autómata celular.
*   Creación de la malla de la cueva utilizando el algoritmo Marching Squares.
*   Paralelización de la generación y procesamiento con **MPICH**.
*   Visualización del resultado con SFML.

## Requisitos Previos

Antes de compilar y ejecutar **ParallelCavesMPI**, necesitarás tener instaladas las siguientes dependencias en tu sistema:

1.  **Un Compilador C++ Moderno:**
    *   GCC/G++ (versión que soporte C++17 o superior)
    *   Clang (versión que soporte C++17 o superior)
    *   MSVC (Visual Studio 2019 o superior, si estás en Windows y quieres usarlo)

2.  **CMake:**
    *   Versión **3.15 o superior**.
    *   Descarga e instala desde [cmake.org](https://cmake.org/download/).

3.  **SFML (Simple and Fast Multimedia Library):**
    *   Versión **3.0.1** (o la última versión de SFML 3).
    *   **Recomendación:** La forma más sencilla y recomendada de instalar SFML 3 es utilizando un gestor de paquetes como **vcpkg**:
        1.  Instala vcpkg: Sigue las instrucciones en [vcpkg.io](https://vcpkg.io/en/getting-started.html).
        2.  Instala SFML 3 con vcpkg:
            ```bash
            vcpkg install sfml
            # Ejemplo específico para Windows x64:
            # vcpkg install sfml[core,graphics,window,system]:x64-windows --triplet=x64-windows
            ```
        3.  Integra vcpkg con tu sistema o CMake:
            ```bash
            vcpkg integrate install
            ```
            Esto te proporcionará una ruta para `CMAKE_TOOLCHAIN_FILE` que podrías necesitar al configurar con CMake.
    *   **Alternativa:** Puedes compilar SFML 3.0.1 desde el código fuente. Visita el [repositorio de SFML en GitHub](https://github.com/SFML/SFML/releases) para obtener el código fuente y sigue sus instrucciones de compilación con CMake.

4.  **Implementación de MPI - MPICH:**
    *   Se requiere **MPICH versión 4.1.2 (o la versión 4.x más reciente, o la "4.3.0" si es una versión específica que posees)**.
    *   **Linux:**
        *   Suelen estar disponibles en los repositorios de tu distribución.
        *   Ejemplo para Debian/Ubuntu:
            ```bash
            sudo apt update
            sudo apt install mpich libmpich-dev
            ```
        *   Ejemplo para Fedora:
            ```bash
            sudo dnf install mpich mpich-devel
            ```
        *   **Desde fuente (recomendado para obtener versiones específicas o más recientes):** Descarga desde [MPICH Downloads](https://www.mpich.org/downloads/) y sigue sus instrucciones de compilación e instalación (generalmente `./configure`, `make`, `sudo make install`).
    *   **macOS:**
        *   La forma más sencilla es a través de Homebrew:
            ```bash
            brew install mpich
            ```
        *   **Desde fuente:** Similar a Linux, descarga y compila.
    *   **Windows:**
        *   **Microsoft MPI (MS-MPI):** Aunque este proyecto especifica MPICH, MS-MPI es una opción común y compatible con el estándar MPI para Windows. Puedes usarlo si la instalación de MPICH nativo en Windows es compleja.
        *   **MPICH para Windows:** Históricamente, Intel MPI o Microsoft MPI eran más comunes en Windows. Para MPICH, revisa el sitio oficial para ver si ofrecen binarios para Windows o si necesitas compilarlo usando Cygwin o MinGW, lo cual puede ser avanzado.
        *   **WSL (Windows Subsystem for Linux):** La forma más sencilla de usar MPICH en Windows es instalarlo dentro de tu entorno WSL siguiendo las instrucciones de Linux.

    Asegúrate de que los comandos de MPICH como `mpicxx` (o `mpicc`, `mpifort`) y `mpiexec` (o `mpirun`) estén disponibles en tu `PATH` después de la instalación.

## Compilación

Una vez que tengas todas las dependencias instaladas:

1.  **Clona el repositorio:**
    ```bash
    git clone https://github.com/TU_USUARIO/ParallelCavesMPI.git
    cd ParallelCavesMPI
    ```
    *(Reemplaza `TU_USUARIO` con tu nombre de usuario y el nombre del repositorio si es diferente).*

2.  **Configura el proyecto con CMake:**
    Crea un directorio de compilación y ejecuta CMake desde allí:
    ```bash
    mkdir build
    cd build
    ```
    *   **Si usaste `vcpkg integrate install`:**
        ```bash
        cmake ..
        ```
    *   **Si necesitas especificar la toolchain de vcpkg manualmente** (reemplaza `[RUTA_A_VCPKG]` con la ruta real a tu instalación de vcpkg):
        ```bash
        cmake -DCMAKE_TOOLCHAIN_FILE=[RUTA_A_VCPKG]/scripts/buildsystems/vcpkg.cmake ..
        ```
        *Ejemplo para Visual Studio en Windows (si usas MS-MPI y vcpkg):*
        ```bash
        cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
        ```

3.  **Compila el proyecto:**
    ```bash
    cmake --build .
    ```
    O, si se generaron Makefiles (Linux/macOS):
    ```bash
    make
    ```
    En Visual Studio, puedes abrir la solución (`.sln`) generada en la carpeta `build` y compilar desde el IDE.

## Ejecución

El ejecutable se encontrará en el directorio `build` (o en un subdirectorio como `build/Debug` o `build/Release` dependiendo de tu configuración de CMake).

Para ejecutar el programa utilizando MPICH (por ejemplo, con 4 procesos):
```bash
cd build  # Asegúrate de estar en el directorio de compilación
mpiexec -n 4 ./ParallelCavesMPI
# o, dependiendo de tu instalación de MPICH:
# mpirun -np 4 ./ParallelCavesMPI
```
Solo el proceso MPI con rango 0 renderizará la ventana SFML. Los otros procesos participarán en los cálculos.

## Estructura del Proyecto (Simplificada)

```
ParallelCavesMPI/
├── CMakeLists.txt        # Script de compilación para CMake
├── README.md             # Este archivo
├── src/                  # Directorio del código fuente
│   ├── main.cpp          # Punto de entrada, lógica MPI, renderizado SFML
│   ├── CaveGenerator.hpp # Definición de la clase para generar el mapa
│   ├── CaveGenerator.cpp # Implementación de CaveGenerator
│   ├── MarchingSquares.hpp # Definición de la clase para Marching Squares
│   └── MarchingSquares.cpp # Implementación de Marching Squares
└── .gitignore            # Archivos a ignorar por Git
```

## Contribuir

Las contribuciones son bienvenidas. Por favor, abre un issue para discutir cambios importantes o reportar bugs. Si deseas contribuir con código, por favor haz un fork del repositorio y envía un Pull Request.

## Licencia

Este proyecto está bajo la Licencia MIT. Ver el archivo `LICENSE` para más detalles (puedes añadir un archivo LICENSE.md si lo deseas).
```

**Cambios Clave Realizados:**

*   Se menciona **MPICH** en lugar de Open MPI.
*   Se indica la versión de MPICH que mencionaste (o la más reciente estable si esa no existe públicamente).
*   Las instrucciones de instalación para MPI se han adaptado para MPICH (comandos de `apt`, `dnf`, `brew`, y la sugerencia de compilar desde fuente).
*   Se ha mencionado `mpiexec` como el comando primario para MPICH, aunque `mpirun` también suele funcionar como un alias o wrapper.

Este README debería ser más preciso para tu configuración deseada. Si "MPICH 4.3.0" es una versión muy específica que obtuviste de una fuente particular, podrías añadir un enlace a esa fuente en la sección de instalación de MPICH.