# Nombre del Ejecutable
TARGET = ParallelCavesMPI

# Compilador C++ para MPI (debe estar en tu PATH)
# Si usas MPICH o OpenMPI, mpicxx es el wrapper estándar.
CXX = mpicxx

# Directorios de código fuente
SRC_DIR = src
OBJ_DIR = obj
# No tenemos un directorio ./include separado en la estructura previa,
# los .hpp están en src/. Si tienes un ./include, ajústalo.
# IDIR = ./include

# --- Flags de Compilación y Enlace ---
# CXXFLAGS: Flags para el compilador C++
#   -std=c++17: Usa el estándar C++17
#   -Wall: Habilita la mayoría de las advertencias
#   -Wextra: Habilita advertencias adicionales
#   -g: Incluye información de depuración (para gdb)
#   -O2: Nivel de optimización 2 (puedes quitar -g y usar -O2 o -O3 para release)
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# LDFLAGS: Flags para el enlazador
LDFLAGS =

# --- Detección del Sistema Operativo y Arquitectura ---
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# --- Configuración de SFML 3.0.1 ---
# El usuario DEBE verificar estas rutas o sobreescribirlas si son diferentes.
# Ejemplo para macOS M1 con Homebrew y SFML 3.0.1 (la versión específica en la ruta es importante)
# Si la versión o la ruta de Homebrew cambian, esto necesita ajuste.
ifeq ($(UNAME_S),Darwin) # macOS
    ifeq ($(UNAME_M),arm64) # Apple Silicon M1/M2/M3
        SFML_BASE_DIR ?= /opt/homebrew/Cellar/sfml/3.0.1
    else # Intel macOS
        SFML_BASE_DIR ?= /usr/local/Cellar/sfml/3.0.1
    endif
else # Linux (ejemplo, puede variar enormemente)
    SFML_BASE_DIR ?= /usr/local # O /usr si se instaló globalmente
endif

SFML_INCLUDE_DIR ?= $(SFML_BASE_DIR)/include
SFML_LIB_DIR ?= $(SFML_BASE_DIR)/lib

# Para SFML 3.x, los nombres de las bibliotecas son típicamente 'sfml-<modulo>'
# y no usan el prefijo '-l' directamente si enlazas el archivo .a o .so.
# Sin embargo, si están en una ruta de biblioteca estándar y se llaman libsfml-*.so,
# entonces -lsfml-* funciona.
# Dado que SFML 3 usa targets de CMake, el enlace manual es menos estándar.
# Probaremos con los nombres comunes de SFML 2.x para simplicidad,
# pero esto es lo MÁS PROBABLE QUE NECESITE AJUSTE para SFML 3.
#
# OPCIÓN 1: Usando -l (asume que los archivos son libsfml-graphics.so, etc. y están en SFML_LIB_DIR)
# Esto es más parecido a SFML 2.x y al ejemplo que diste.
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system
#
# OPCIÓN 2: Enlazando archivos directamente (más robusto para SFML 3 si no hay -l)
# Esto requeriría saber si son .a (estático) o .so/.dylib (dinámico)
# y el nombre exacto, ej: $(SFML_LIB_DIR)/libsfml-graphics.dylib
# SFML_GRAPHICS_LIB = $(SFML_LIB_DIR)/libsfml-graphics.dylib # o .so o .a
# SFML_WINDOW_LIB = $(SFML_LIB_DIR)/libsfml-window.dylib
# SFML_SYSTEM_LIB = $(SFML_LIB_DIR)/libsfml-system.dylib
# ALL_SFML_LIBS = $(SFML_GRAPHICS_LIB) $(SFML_WINDOW_LIB) $(SFML_SYSTEM_LIB)

# --- Configuración de MPI ---
# mpicxx se encarga de los flags de MPI, así que generalmente no necesitamos
# especificar MPI_CFLAGS o MPI_LIBS aquí si CXX = mpicxx.

# --- Agregado de Inclusiones y Bibliotecas ---
# Si tienes headers en ./src o ./include:
# LOCAL_INCLUDES = -I$(SRC_DIR) # Si los .hpp están junto a los .cpp
# LOCAL_INCLUDES = -I$(IDIR)   # Si tienes un ./include separado
LOCAL_INCLUDES = -I$(SRC_DIR) # Asumiendo .hpp en src/ como en la estructura previa

INCLUDES = $(LOCAL_INCLUDES) -I$(SFML_INCLUDE_DIR)
LDFLAGS += -L$(SFML_LIB_DIR) # Para que el enlazador encuentre las bibliotecas SFML
LIBS = $(SFML_LIBS)          # Usaremos la OPCIÓN 1 por ahora, la más parecida al ejemplo

# En macOS, a veces se necesitan frameworks para SFML, aunque SFML 3 puede manejarlos mejor.
# Si hay errores de enlace relacionados con símbolos de Cocoa, OpenGL, etc., descomenta:
ifeq ($(UNAME_S),Darwin)
    # LIBS += -framework Cocoa -framework OpenGL -framework IOKit -framework Carbon -framework AudioUnit -framework CoreAudio -framework AVFoundation
endif

# --- Definiciones del Proyecto ---
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# --- Reglas del Makefile ---
.PHONY: all clean run help

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "LD -> $@"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS) # $^ son todos los OBJS
	@echo "Ejecutable '$@' creado exitosamente."

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "CXX -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Limpiando archivos compilados..."
	rm -f $(OBJ_DIR)/*.o $(TARGET)
	@echo "Limpieza completada."

run: all
	@echo "Ejecutando $(TARGET) con 4 procesos MPI..."
	mpiexec -n 4 ./$(TARGET) # Usando mpiexec como se discutió para MPICH

help:
	@echo "Uso: make [target]"
	@echo "Targets disponibles:"
	@echo "  all     : Compila el proyecto (default)."
	@echo "  run     : Compila y ejecuta el proyecto con 'mpiexec -n 4'."
	@echo "  clean   : Elimina los archivos generados por la compilación."
	@echo "  help    : Muestra este mensaje de ayuda."
	@echo ""
	@echo "Este Makefile asume que SFML 3.0.1 está instalado en una ruta conocida"
	@echo "(ej. /opt/homebrew/Cellar/sfml/3.0.1 en macOS M1)."
	@echo "Si tu instalación de SFML está en otro lugar, puedes sobreescribir las rutas:"
	@echo "  make SFML_BASE_DIR=/custom/path/to/sfml-3.0.1"
	@echo "o individualmente:"
	@echo "  make SFML_INCLUDE_DIR=/custom/include SFML_LIB_DIR=/custom/lib"
	@echo "Asegúrate de que mpicxx (de MPICH) esté en tu PATH."