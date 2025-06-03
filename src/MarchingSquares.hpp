#ifndef MARCHING_SQUARES_HPP
#define MARCHING_SQUARES_HPP

#include <SFML/System/Vector2.hpp>
#include <vector>

#include "CaveGenerator.hpp"  // Para Grid

// Estructura para un segmento de línea
struct LineSegment
{
    sf::Vector2f start;
    sf::Vector2f end;
};

class MarchingSquares
{
   public:
    MarchingSquares(float squareSize);
    std::vector<LineSegment> generateMesh(const Grid &subGrid, int globalOffsetY = 0);

   private:
    float TILE_SIZE;
    sf::Vector2f interpolateVertex(sf::Vector2f p1, sf::Vector2f p2, float val1, float val2);
    // Puntos de control para las esquinas de un cuadrado de Marching Squares
    struct ControlNode
    {
        sf::Vector2f position;
        bool active;  // Es pared?
        ControlNode(sf::Vector2f pos, bool act) : position(pos), active(act)
        {
        }
    };

    struct Square
    {
        ControlNode topLeft, topRight, bottomRight, bottomLeft;
        // Nodos en los puntos medios de los lados (para la interpolación)
        sf::Vector2f top, right, bottom, left;
    };

    int getConfiguration(const Square &square);
    void processSquare(const Square &square, std::vector<LineSegment> &lines);
};

#endif  // MARCHING_SQUARES_HPP
