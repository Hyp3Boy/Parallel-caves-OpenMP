#ifndef CAVE_GENERATOR_HPP
#define CAVE_GENERATOR_HPP

#include <random>
#include <vector>

using GridRow = std::vector<int>;
using LocalGrid = std::vector<GridRow>; 

class CaveGenerator
{
   public:
    CaveGenerator(int gridWidth, int gridHeight, unsigned int seed);

    void initializeMap(float fillProbability);
    void smoothMapIteration();

    const LocalGrid &getLocalGrid() const;
    int getWidth() const
    {
        return mapWidth;
    }
    int getHeight() const
    {
        return mapHeight;
    }  
   private:
    int mapWidth;
    int mapHeight;  
    LocalGrid grid;
    std::mt19937 generator;

    int countAliveNeighbours(int x, int y);
};

#endif 
