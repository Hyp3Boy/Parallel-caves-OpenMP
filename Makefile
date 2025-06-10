# Nombre del ejecutable final (debe coincidir con el de CMakeLists.txt)
TARGET_NAME = ParallelCavesOMP
BUILD_DIR = build
EXECUTABLE = $(BUILD_DIR)/bin/$(TARGET_NAME)

# --- Detección del Sistema Operativo para elegir el compilador ---
# Por defecto, no pasamos flags especiales de compilador
CMAKE_COMPILER_FLAGS =

# Si el sistema es Darwin (macOS)...
ifeq ($(shell uname -s), Darwin)
	# ...le decimos a CMake que use Clang explícitamente.
	CMAKE_COMPILER_FLAGS = -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++
endif

# .PHONY asegura que estos "targets" se ejecuten siempre,
# incluso si existe un archivo con el mismo nombre.
.PHONY: all build run clean help

# El target por defecto: si solo escribes 'make', compilará.
all: build

# Compila el proyecto.
# Usamos los flags de compilador que definimos arriba.
build:
	@echo "--- Configurando y Compilando con CMake ---"
	@cmake -S . -B $(BUILD_DIR) $(CMAKE_COMPILER_FLAGS)
	@cmake --build $(BUILD_DIR)

# El comando que realmente quieres: compila y luego ejecuta.
run: build
	@echo "--- Ejecutando el Proyecto ---"
	@$(EXECUTABLE)

# Limpia todos los archivos generados por CMake.
clean:
	@echo "--- Limpiando el Directorio de Compilación ---"
	@rm -rf $(BUILD_DIR)

# Un pequeño menú de ayuda.
help:
	@echo "Uso: make [target]"
	@echo "  all      Compila el proyecto (default)."
	@echo "  build    Compila el proyecto."
	@echo "  run      Compila si es necesario y luego ejecuta el proyecto."
	@echo "  clean    Elimina todos los archivos generados."