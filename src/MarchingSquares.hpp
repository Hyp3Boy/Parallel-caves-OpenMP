#ifndef MARCHING_SQUARES_HPP
#define MARCHING_SQUARES_HPP

#include <SFML/System/Vector2.hpp>
#include <vector>

#include "CaveGenerator.hpp"

struct LineSegment
{
    sf::Vector2f start;
    sf::Vector2f end;
};

class MarchingSquares
{
   public:
    MarchingSquares(float squareSize);
    std::vector<LineSegment> generateMesh(const LocalGrid &grid);

   private:
    float TILE_SIZE;

    struct ControlNode
    {
        sf::Vector2f position;
        bool active;
        ControlNode(sf::Vector2f pos, bool act) : position(pos), active(act)
        {
        }
    };

    struct Square
    {
        ControlNode topLeft, topRight, bottomRight, bottomLeft;
        sf::Vector2f top, right, bottom, left;
    };

    int getConfiguration(const Square &square);
    void processSquare(const Square &mSquare, int config, std::vector<LineSegment> &lines);
};

#endif
