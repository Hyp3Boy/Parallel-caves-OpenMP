# --- OMP-CHANGE: Cambiamos el nombre del ejecutable ---
TARGET = ParallelCavesOMP

# --- OMP-CHANGE: Cambiamos el compilador de MPI a Clang++ estándar de macOS ---
CXX = clang++

# Directorios de código fuente (sin cambios)
SRC_DIR = src
OBJ_DIR = obj
LOCAL_INCLUDES = -I$(SRC_DIR)

# --- Flags de Compilación y Enlace ---
# CXXFLAGS: Flags base para el compilador C++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2 # -g para depurar, -O2 para optimizar

# LDFLAGS: Flags base para el enlazador
LDFLAGS =

# --- Detección del Sistema Operativo y Arquitectura (sin cambios) ---
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# --- OMP-CHANGE: Configuración de OpenMP para macOS con libomp de Homebrew ---
# Asumimos que libomp fue instalado con 'brew install libomp'
# Usamos 'brew --prefix' para encontrar la ruta de instalación automáticamente.
OMP_BASE_DIR ?= $(shell brew --prefix libomp)
OMP_CFLAGS = -Xpreprocessor -fopenmp -I$(OMP_BASE_DIR)/include
OMP_LDFLAGS = -L$(OMP_BASE_DIR)/lib
OMP_LIBS = -lomp

# Agregamos los flags de OpenMP a las variables globales
CXXFLAGS += $(OMP_CFLAGS)
LDFLAGS += $(OMP_LDFLAGS)


# --- Configuración de SFML 3.0.1 (sin cambios, esta lógica es robusta) ---
# El usuario DEBE verificar estas rutas o sobreescribirlas si son diferentes.
ifeq ($(UNAME_S),Darwin) # macOS
    ifeq ($(UNAME_M),arm64) # Apple Silicon M1/M2/M3
        SFML_BASE_DIR ?= /opt/homebrew/Cellar/sfml/3.0.1
    else # Intel macOS
        SFML_BASE_DIR ?= /usr/local/Cellar/sfml/3.0.1
    endif
else # Linux (ejemplo)
    SFML_BASE_DIR ?= /usr/local
endif

SFML_INCLUDE_DIR ?= $(SFML_BASE_DIR)/include
SFML_LIB_DIR ?= $(SFML_BASE_DIR)/lib
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system


# --- Agregado de Inclusiones y Bibliotecas ---
INCLUDES = $(LOCAL_INCLUDES) -I$(SFML_INCLUDE_DIR)
LDFLAGS += -L$(SFML_LIB_DIR)

# Combinamos las bibliotecas de SFML y OpenMP
LIBS = $(SFML_LIBS) $(OMP_LIBS)

# Frameworks de macOS para SFML (sin cambios)
ifeq ($(UNAME_S),Darwin)
    LIBS += -framework Cocoa -framework OpenGL -framework IOKit -framework Carbon -framework AudioUnit -framework CoreAudio -framework AVFoundation
endif

# --- Definiciones del Proyecto (sin cambios) ---
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# --- Reglas del Makefile ---
.PHONY: all clean run help

all: $(TARGET)

# --- OMP-CHANGE: La regla de enlace ahora usa CXX=clang++ y los LDFLAGS/LIBS actualizados ---
$(TARGET): $(OBJS)
	@echo "LD -> $@"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Ejecutable '$@' creado exitosamente."

# --- OMP-CHANGE: La regla de compilación ahora usa CXX=clang++ y los CXXFLAGS actualizados ---
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	@echo "CXX -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Limpiando archivos compilados..."
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "Limpieza completada."

# --- OMP-CHANGE: El comando 'run' ya no usa mpiexec ---
run: all
	@echo "Ejecutando $(TARGET)..."
	./$(TARGET)

# --- OMP-CHANGE: El mensaje de ayuda ha sido actualizado para OpenMP ---
help:
	@echo "Uso: make [target]"
	@echo "Targets disponibles:"
	@echo "  all     : Compila el proyecto (default)."
	@echo "  run     : Compila y ejecuta el proyecto."
	@echo "  clean   : Elimina los archivos generados por la compilación."
	@echo "  help    : Muestra este mensaje de ayuda."
	@echo ""
	@echo "Este Makefile está configurado para compilar con OpenMP en macOS."
	@echo "Dependencias requeridas (vía Homebrew): 'brew install sfml libomp'"
	@echo ""
	@echo "El Makefile intenta encontrar SFML y libomp automáticamente."
	@echo "Si tus instalaciones están en rutas no estándar, puedes sobreescribirlas:"
	@echo "  make SFML_BASE_DIR=/ruta/a/sfml OMP_BASE_DIR=/ruta/a/libomp"