#include "MarchingSquares.hpp"

#include <SFML/Graphics/Vertex.hpp>  // Para sf::Vertex y sf::VertexArray si lo usas directamente

MarchingSquares::MarchingSquares(float squareSize) : TILE_SIZE(squareSize)
{
}

// Esta es una implementación MUY básica y conceptual de Marching Squares.
// Necesitarás consultar algoritmos completos para todos los 16 casos.
std::vector<LineSegment> MarchingSquares::generateMesh(const Grid &subGrid, int globalOffsetY)
{
    std::vector<LineSegment> lines;
    if (subGrid.empty() || subGrid[0].empty())
        return lines;

    int rows = subGrid.size();
    int cols = subGrid[0].size();

    for (int y = 0; y < rows - 1; ++y)
    {
        for (int x = 0; x < cols - 1; ++x)
        {
            // Definir los 4 puntos de control de la celda actual
            // Los valores '1' de la grid son paredes, '0' son espacio abierto.
            // Marching Squares usualmente define "activo" como estar DENTRO del objeto.
            // Aquí, si el valor de la grid es 1 (pared), lo consideramos "inactivo" para el espacio
            // abierto. O, si lo prefieres, considera que "activo" es ser una pared. Vamos a asumir
            // que generamos líneas para el contorno de las paredes. Entonces, un nodo está "activo"
            // si es parte de una pared.

            Square mSquare = {
                    /*topLeft*/ ControlNode(
                            {(float) x * TILE_SIZE, (float) (y + globalOffsetY) * TILE_SIZE},
                            subGrid[y][x] == 1),
                    /*topRight*/
                    ControlNode(
                            {(float) (x + 1) * TILE_SIZE, (float) (y + globalOffsetY) * TILE_SIZE},
                            subGrid[y][x + 1] == 1),
                    /*bottomRight*/
                    ControlNode({(float) (x + 1) * TILE_SIZE,
                                 (float) (y + 1 + globalOffsetY) * TILE_SIZE},
                                subGrid[y + 1][x + 1] == 1),
                    /*bottomLeft*/
                    ControlNode(
                            {(float) x * TILE_SIZE, (float) (y + 1 + globalOffsetY) * TILE_SIZE},
                            subGrid[y + 1][x] == 1)};

            // Calcular puntos medios (simplificado, sin interpolación real aquí)
            mSquare.top = sf::Vector2f(mSquare.topLeft.position.x + TILE_SIZE / 2.f,
                                       mSquare.topLeft.position.y);
            mSquare.right = sf::Vector2f(mSquare.topRight.position.x,
                                         mSquare.topRight.position.y + TILE_SIZE / 2.f);
            mSquare.bottom = sf::Vector2f(mSquare.bottomLeft.position.x + TILE_SIZE / 2.f,
                                          mSquare.bottomLeft.position.y);
            mSquare.left = sf::Vector2f(mSquare.topLeft.position.x,
                                        mSquare.topLeft.position.y + TILE_SIZE / 2.f);

            int config = getConfiguration(mSquare);
            processSquare(mSquare, config, lines);
        }
    }
    return lines;
}

int MarchingSquares::getConfiguration(const Square &square)
{
    int configuration = 0;
    if (square.topLeft.active)
        configuration |= 8;  // 1000
    if (square.topRight.active)
        configuration |= 4;  // 0100
    if (square.bottomRight.active)
        configuration |= 2;  // 0010
    if (square.bottomLeft.active)
        configuration |= 1;  // 0001
    return configuration;
}

// Necesitarás implementar los 16 casos de Marching Squares
void MarchingSquares::processSquare(const Square &mSquare, int config,
                                    std::vector<LineSegment> &lines)
{
    // Ejemplo para un caso simple (esquina superior izquierda activa)
    switch (config)
    {
        case 0:
            break;  // No lines
        case 1:
            lines.push_back({mSquare.left, mSquare.bottom});
            break;  // bottom-left
        case 2:
            lines.push_back({mSquare.bottom, mSquare.right});
            break;  // bottom-right
        case 3:
            lines.push_back({mSquare.left, mSquare.right});
            break;  // bottom-left & bottom-right
        case 4:
            lines.push_back({mSquare.top, mSquare.right});
            break;  // top-right
        // ... y así sucesivamente para los 16 casos (o 15 sin el 0)
        // Casos como el 5 (top-right y bottom-left) pueden necesitar dos líneas o manejo de
        // ambigüedad.
        case 5:  // top-right and bottom-left (ambiguous or two lines)
            lines.push_back({mSquare.top, mSquare.right});
            lines.push_back({mSquare.left, mSquare.bottom});
            break;
        case 6:
            lines.push_back({mSquare.top, mSquare.bottom});
            break;  // top-right & bottom-right
        case 7:
            lines.push_back({mSquare.left, mSquare.top});
            break;  // all but top-left
        case 8:
            lines.push_back({mSquare.top, mSquare.left});
            break;  // top-left
        case 9:
            lines.push_back({mSquare.top, mSquare.bottom});
            break;  // top-left & bottom-left
        case 10:    // top-left and bottom-right (ambiguous or two lines)
            lines.push_back({mSquare.top, mSquare.left});
            lines.push_back({mSquare.bottom, mSquare.right});
            break;
        case 11:
            lines.push_back({mSquare.top, mSquare.right});
            break;  // all but top-right
        case 12:
            lines.push_back({mSquare.left, mSquare.right});
            break;  // top-left & top-right
        case 13:
            lines.push_back({mSquare.bottom, mSquare.right});
            break;  // all but bottom-right
        case 14:
            lines.push_back({mSquare.bottom, mSquare.left});
            break;  // all but bottom-left
        case 15:
            break;  // All active, no lines (or lines around border if that's the convention)
        default:
            break;
    }
}
