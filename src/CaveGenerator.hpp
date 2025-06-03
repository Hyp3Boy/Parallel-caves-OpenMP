#ifndef CAVE_GENERATOR_HPP
#define CAVE_GENERATOR_HPP

#include <random>  // Para el generador
#include <vector>

// Representa el mapa de la cueva como una cuadrícula de enteros (1 = pared, 0 = suelo)
using GridRow = std::vector<int>;
using LocalGrid = std::vector<GridRow>;

class CaveGenerator
{
   public:
    // Constructor para una porción local del mapa
    CaveGenerator(int gridWidth, int localGridHeight, unsigned int globalSeed,
                  int globalStartRowOffset);

    void initializeMap(float fillProbability);
    // Realiza una iteración de suavizado usando las filas halo proporcionadas
    void smoothMapIteration(const GridRow &topHalo, const GridRow &bottomHalo);

    const LocalGrid &getLocalGrid() const;
    GridRow getFirstRowData() const;  // Para enviar como halo inferior al proceso anterior
    GridRow getLastRowData() const;   // Para enviar como halo superior al proceso siguiente
    int getWidth() const
    {
        return mapWidth;
    }
    int getLocalHeight() const
    {
        return localMapHeight;
    }

   private:
    int mapWidth;
    int localMapHeight;  // Altura de la porción de este proceso
    LocalGrid grid;
    std::mt19937 generator;  // Generador de números aleatorios

    int countAliveNeighbours(int x, int y, const GridRow &topHalo, const GridRow &bottomHalo);
};

#endif  // CAVE_GENERATOR_HPP
