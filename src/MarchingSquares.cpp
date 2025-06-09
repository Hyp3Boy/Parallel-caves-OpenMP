#include "MarchingSquares.hpp"

#include <omp.h>

MarchingSquares::MarchingSquares(float squareSize) : TILE_SIZE(squareSize)
{
}

std::vector<LineSegment> MarchingSquares::generateMesh(const LocalGrid &grid)
{
    if (grid.empty() || grid[0].empty())
        return {};

    std::vector<LineSegment> allLines;
    int rows = grid.size();
    int cols = grid[0].size();

#pragma omp parallel
    {
        std::vector<LineSegment> localLines;

#pragma omp for
        for (int y = 0; y < rows - 1; ++y)
        {
            for (int x = 0; x < cols - 1; ++x)
            {
                Square mSquare = {
                        /*topLeft*/ ControlNode({(float) x * TILE_SIZE, (float) y * TILE_SIZE},
                                                grid[y][x] == 1),
                        /*topRight*/
                        ControlNode({(float) (x + 1) * TILE_SIZE, (float) y * TILE_SIZE},
                                    grid[y][x + 1] == 1),
                        /*bottomRight*/
                        ControlNode({(float) (x + 1) * TILE_SIZE, (float) (y + 1) * TILE_SIZE},
                                    grid[y + 1][x + 1] == 1),
                        /*bottomLeft*/
                        ControlNode({(float) x * TILE_SIZE, (float) (y + 1) * TILE_SIZE},
                                    grid[y + 1][x] == 1)};

                mSquare.top = sf::Vector2f(mSquare.topLeft.position.x + TILE_SIZE / 2.f,
                                           mSquare.topLeft.position.y);
                mSquare.right = sf::Vector2f(mSquare.topRight.position.x,
                                             mSquare.topRight.position.y + TILE_SIZE / 2.f);
                mSquare.bottom = sf::Vector2f(mSquare.bottomLeft.position.x + TILE_SIZE / 2.f,
                                              mSquare.bottomLeft.position.y);
                mSquare.left = sf::Vector2f(mSquare.topLeft.position.x,
                                            mSquare.topLeft.position.y + TILE_SIZE / 2.f);

                int config = getConfiguration(mSquare);
                processSquare(mSquare, config, localLines);
            }
        }

#pragma omp critical
        {
            allLines.insert(allLines.end(), localLines.begin(), localLines.end());
        }
    }
    return allLines;
}

int MarchingSquares::getConfiguration(const Square &square)
{
    int configuration = 0;
    if (square.topLeft.active)
        configuration |= 8;
    if (square.topRight.active)
        configuration |= 4;
    if (square.bottomRight.active)
        configuration |= 2;
    if (square.bottomLeft.active)
        configuration |= 1;
    return configuration;
}

void MarchingSquares::processSquare(const Square &mSquare, int config,
                                    std::vector<LineSegment> &lines)
{
    switch (config)
    {
        case 0:
            break;
        case 1:
            lines.push_back({mSquare.left, mSquare.bottom});
            break;
        case 2:
            lines.push_back({mSquare.bottom, mSquare.right});
            break;
        case 3:
            lines.push_back({mSquare.left, mSquare.right});
            break;
        case 4:
            lines.push_back({mSquare.top, mSquare.right});
            break;
        case 5:
            lines.push_back({mSquare.top, mSquare.right});
            lines.push_back({mSquare.left, mSquare.bottom});
            break;
        case 6:
            lines.push_back({mSquare.top, mSquare.bottom});
            break;
        case 7:
            lines.push_back({mSquare.left, mSquare.top});
            break;
        case 8:
            lines.push_back({mSquare.top, mSquare.left});
            break;
        case 9:
            lines.push_back({mSquare.top, mSquare.bottom});
            break;
        case 10:
            lines.push_back({mSquare.top, mSquare.left});
            lines.push_back({mSquare.bottom, mSquare.right});
            break;
        case 11:
            lines.push_back({mSquare.top, mSquare.right});
            break;
        case 12:
            lines.push_back({mSquare.left, mSquare.right});
            break;
        case 13:
            lines.push_back({mSquare.bottom, mSquare.right});
            break;
        case 14:
            lines.push_back({mSquare.bottom, mSquare.left});
            break;
        case 15:
            break;
        default:
            break;
    }
}
