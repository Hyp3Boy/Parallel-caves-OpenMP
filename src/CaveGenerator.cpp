#include "CaveGenerator.hpp"

#include <omp.h>

CaveGenerator::CaveGenerator(int gridWidth, int gridHeight, unsigned int seed)
    : mapWidth(gridWidth), mapHeight(gridHeight)
{
    grid.resize(mapHeight, GridRow(mapWidth));
    generator.seed(seed);
}

void CaveGenerator::initializeMap(float fillProbability)
{
    std::vector<std::mt19937> thread_generators;
    unsigned int main_seed = generator();
#pragma omp parallel
    {
#pragma omp single
        {
            thread_generators.resize(omp_get_num_threads());
            for (int i = 0; i < omp_get_num_threads(); ++i)
            {
                thread_generators[i].seed(main_seed + i);
            }
        }
    }

#pragma omp parallel for
    for (int y = 0; y < mapHeight; ++y)
    {
        int thread_id = omp_get_thread_num();
        std::uniform_real_distribution<> dis(0.0, 1.0);
        for (int x = 0; x < mapWidth; ++x)
        {
            grid[y][x] = (dis(thread_generators[thread_id]) < fillProbability) ? 1 : 0;
        }
    }
}

void CaveGenerator::smoothMapIteration()
{
    LocalGrid newGrid = grid;

#pragma omp parallel for collapse(2)
    for (int y = 0; y < mapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            // Mantenemos la lógica de bordes duros
            if (x == 0 || x == mapWidth - 1 || y == 0 || y == mapHeight - 1)
            {
                newGrid[y][x] = 1;
                continue;
            }

            int neighbours = countAliveNeighbours(x, y);

            if (neighbours > 4)
                newGrid[y][x] = 1;
            else if (neighbours < 4)
                newGrid[y][x] = 0;
        }
    }
    grid = newGrid;
}

int CaveGenerator::countAliveNeighbours(int x, int y)
{
    int wallCount = 0;
    for (int ny = y - 1; ny <= y + 1; ++ny)
    {
        for (int nx = x - 1; nx <= x + 1; ++nx)
        {
            if (nx == x && ny == y)
                continue;

            // Si está fuera de los límites, se considera una pared.
            if (nx < 0 || nx >= mapWidth || ny < 0 || ny >= mapHeight)
            {
                wallCount++;
            }
            else
            {
                wallCount += grid[ny][nx];
            }
        }
    }
    return wallCount;
}

const LocalGrid &CaveGenerator::getLocalGrid() const
{
    return grid;
}