#include "CaveGenerator.hpp"

#include <algorithm>  // Para std::fill

CaveGenerator::CaveGenerator(int gridWidth, int localGridHeight, unsigned int globalSeed,
                             int globalStartRowOffset)
    : mapWidth(gridWidth), localMapHeight(localGridHeight)
{
    grid.resize(localMapHeight, GridRow(mapWidth));
    // Cada proceso inicializa su generador con la semilla global + un offset
    // para asegurar que cada uno genere una parte diferente pero determinista.
    // globalStartRowOffset asegura que las filas globales tengan la misma aleatoriedad
    // independientemente de cómo se dividan entre procesos.
    generator.seed(globalSeed);
    // "Avanzamos" el generador para esta porción.
    // Cada celda usa un número aleatorio.
    if (globalStartRowOffset > 0)
    {
        generator.discard(static_cast<unsigned long long>(globalStartRowOffset) * mapWidth);
    }
}

void CaveGenerator::initializeMap(float fillProbability)
{
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int y = 0; y < localMapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            // Condición de borde global (opcional, si quieres un borde duro en el mapa completo)
            // bool isGlobalBorder = (globalStartRow + y == 0 || globalStartRow + y ==
            // GLOBAL_MAP_HEIGHT - 1 || x == 0 || x == mapWidth - 1); if (isGlobalBorder) grid[y][x]
            // = 1; else
            grid[y][x] = (dis(generator) < fillProbability) ? 1 : 0;
        }
    }
}

void CaveGenerator::smoothMapIteration(const GridRow &topHalo, const GridRow &bottomHalo)
{
    LocalGrid newGrid = grid;  // Copia para trabajar sobre ella

    for (int y = 0; y < localMapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            // No suavizar los bordes globales del mapa si así se desea (ej. x=0, x=mapWidth-1)
            if (x == 0 || x == mapWidth - 1)
            {  // Mantener bordes laterales como paredes
                newGrid[y][x] = 1;
                continue;
            }
            // Si este generador está en el borde superior/inferior GLOBAL y no hay halo real,
            // entonces los bordes globales son manejados por countAliveNeighbours
            // (si topHalo/bottomHalo están vacíos, countAliveNeighbours los trata como paredes).

            int neighbours = countAliveNeighbours(x, y, topHalo, bottomHalo);
            if (neighbours > 4)
                newGrid[y][x] = 1;
            else if (neighbours < 4)
                newGrid[y][x] = 0;
            // else: newGrid[y][x] = grid[y][x]; // Mantiene el estado actual
        }
    }
    grid = newGrid;
}

int CaveGenerator::countAliveNeighbours(int x, int y, const GridRow &topHalo,
                                        const GridRow &bottomHalo)
{
    int wallCount = 0;
    for (int ny = y - 1; ny <= y + 1; ++ny)
    {
        for (int nx = x - 1; nx <= x + 1; ++nx)
        {
            if (nx == x && ny == y)
                continue;  // No contarse a sí mismo

            // Determinar de dónde leer el valor del vecino
            int neighbourValue;
            if (nx < 0 || nx >= mapWidth)
            {                        // Fuera de los límites laterales
                neighbourValue = 1;  // Considerar como pared
            }
            else if (ny < 0)
            {  // Fila superior (necesita topHalo)
                if (!topHalo.empty())
                {
                    neighbourValue = topHalo[nx];
                }
                else
                {
                    neighbourValue = 1;  // No hay halo superior (borde global), considerar pared
                }
            }
            else if (ny >= localMapHeight)
            {  // Fila inferior (necesita bottomHalo)
                if (!bottomHalo.empty())
                {
                    neighbourValue = bottomHalo[nx];
                }
                else
                {
                    neighbourValue = 1;  // No hay halo inferior (borde global), considerar pared
                }
            }
            else
            {  // Dentro de la grid local
                neighbourValue = grid[ny][nx];
            }
            wallCount += neighbourValue;
        }
    }
    return wallCount;
}

const LocalGrid &CaveGenerator::getLocalGrid() const
{
    return grid;
}

GridRow CaveGenerator::getFirstRowData() const
{
    if (localMapHeight > 0)
        return grid[0];
    return {};
}

GridRow CaveGenerator::getLastRowData() const
{
    if (localMapHeight > 0)
        return grid[localMapHeight - 1];
    return {};
}
