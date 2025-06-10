# Versión mínima de CMake. 3.14 es suficiente para FetchContent y OpenMP.
cmake_minimum_required(VERSION 3.14)

# Definición del proyecto: nombre, lenguaje y estándar C++.
project(ParallelCavesOMP LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencia 1: OpenMP ---
# SOLUCIÓN: Lógica para encontrar Homebrew y añadir su ruta a CMake
# Esto es específico para macOS para que el script siga siendo portable.
if(APPLE)
    # Primero, encontramos el ejecutable de 'brew'
    find_program(BREW_EXECUTABLE brew)

    # Si 'brew' existe, lo usamos para encontrar libomp
    if(BREW_EXECUTABLE)
        message(STATUS "Homebrew encontrado. Buscando libomp...")
        execute_process(
            COMMAND "${BREW_EXECUTABLE}" --prefix libomp
            RESULT_VARIABLE BREW_RESULT
            OUTPUT_VARIABLE LIBOMP_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        # Si el comando para encontrar libomp fue exitoso...
        if(BREW_RESULT EQUAL 0)
            message(STATUS "libomp de Homebrew encontrado en: ${LIBOMP_PREFIX}")
            # ...añadimos esa ruta a los lugares donde CMake busca paquetes.
            list(APPEND CMAKE_PREFIX_PATH "${LIBOMP_PREFIX}")
        else()
            message(WARNING "Homebrew está instalado, pero 'brew --prefix libomp' falló. Asegúrate de haber hecho 'brew install libomp'.")
        endif()
    endif()
endif()

# Ahora, find_package debería encontrar OpenMP sin problemas en cualquier sistema.
find_package(OpenMP REQUIRED)


# --- Dependencia 2: SFML 3.0.1 ---
# Usa FetchContent para descargar y preparar SFML automáticamente.
include(FetchContent)
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG        3.0.1
  GIT_SHALLOW    ON
)
FetchContent_MakeAvailable(SFML)


# --- Ficheros del Proyecto ---
# Busca automáticamente todos los archivos .cpp en el directorio 'src'.
file(GLOB_RECURSE SOURCES "src/*.cpp")


# --- Creación del Ejecutable ---
add_executable(ParallelCavesOMP ${SOURCES})


# --- Enlazado de Librerías ---
target_link_libraries(ParallelCavesOMP PRIVATE
    # Enlaza los componentes de SFML que usas
    SFML::Graphics
    SFML::Window
    SFML::System

    # Enlaza OpenMP de forma correcta y multiplataforma
    OpenMP::OpenMP_CXX
)

# --- Configuración de Salida ---
# Coloca el ejecutable final en una carpeta 'bin' para mantener el orden.
set_target_properties(ParallelCavesOMP PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)