#include <omp.h>

#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <optional>
#include <vector>

#include "CaveGenerator.hpp"
#include "MarchingSquares.hpp"

// --- Constantes (sin cambios) ---
const int GLOBAL_MAP_WIDTH = 160;
const int GLOBAL_MAP_HEIGHT = 120;
const float TILE_SIZE_PX = 6.0f;
const int SMOOTHING_ITERATIONS = 5;
const float INITIAL_FILL_PROBABILITY = 0.45f;

// --- Máquina de Estados (sin cambios) ---
enum class AppState
{
    INITIALIZING,
    SMOOTHING,
    MARCHING_SQUARES,
    DONE
};

// CORRECCIÓN: La función ahora crea dos triángulos por cada cuadrado de la rejilla.
void updateGridVisuals(sf::VertexArray &triangles, const LocalGrid &grid)
{
    triangles.clear();
    for (size_t y = 0; y < grid.size(); ++y)
    {
        for (size_t x = 0; x < grid[y].size(); ++x)
        {
            if (grid[y][x] == 1)  // Si es una pared
            {
                // Definimos las 4 esquinas del cuadrado
                sf::Vector2f p1(x * TILE_SIZE_PX, y * TILE_SIZE_PX);              // Top-left
                sf::Vector2f p2((x + 1) * TILE_SIZE_PX, y * TILE_SIZE_PX);        // Top-right
                sf::Vector2f p3((x + 1) * TILE_SIZE_PX, (y + 1) * TILE_SIZE_PX);  // Bottom-right
                sf::Vector2f p4(x * TILE_SIZE_PX, (y + 1) * TILE_SIZE_PX);        // Bottom-left

                // Creamos el primer triángulo (p1, p2, p4)
                triangles.append({p1, sf::Color::White});
                triangles.append({p2, sf::Color::White});
                triangles.append({p4, sf::Color::White});

                // Creamos el segundo triángulo (p2, p3, p4)
                triangles.append({p2, sf::Color::White});
                triangles.append({p3, sf::Color::White});
                triangles.append({p4, sf::Color::White});
            }
        }
    }
}

int main(int argc, char *argv[])
{
    std::cout << "CavaParallelisOMP (Animado)" << std::endl;

    sf::RenderWindow window(sf::VideoMode({(unsigned int) (GLOBAL_MAP_WIDTH * TILE_SIZE_PX),
                                           (unsigned int) (GLOBAL_MAP_HEIGHT * TILE_SIZE_PX)}),
                            "cave generation");
    window.setFramerateLimit(60);

    unsigned int global_seed =
            static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());

    std::optional<CaveGenerator> caveGen;
    MarchingSquares msAlgo(TILE_SIZE_PX);

    sf::VertexArray visuals(sf::PrimitiveType::Triangles);

    AppState currentState = AppState::INITIALIZING;
    int smoothing_step = 0;
    sf::Clock animationClock;
    sf::Time updateInterval = sf::seconds(1.0f);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        if (currentState != AppState::DONE && animationClock.getElapsedTime() > updateInterval)
        {
            switch (currentState)
            {
                case AppState::INITIALIZING:
                    std::cout << "Paso 1: Generando ruido inicial..." << std::endl;
                    caveGen.emplace(GLOBAL_MAP_WIDTH, GLOBAL_MAP_HEIGHT, global_seed);
                    caveGen->initializeMap(INITIAL_FILL_PROBABILITY);
                    updateGridVisuals(visuals, caveGen->getLocalGrid());
                    currentState = AppState::SMOOTHING;
                    break;

                case AppState::SMOOTHING:
                    if (smoothing_step < SMOOTHING_ITERATIONS)
                    {
                        smoothing_step++;
                        std::cout << "Paso 2: Suavizado, iteracion " << smoothing_step << "..."
                                  << std::endl;
                        caveGen->smoothMapIteration();
                        updateGridVisuals(visuals, caveGen->getLocalGrid());
                    }
                    else
                    {
                        currentState = AppState::MARCHING_SQUARES;
                    }
                    break;

                case AppState::MARCHING_SQUARES: {
                    std::cout << "Paso 3: Generando malla con Marching Squares..." << std::endl;
                    std::vector<LineSegment> segments =
                            msAlgo.generateMesh(caveGen->getLocalGrid());

                    // Este cambio a Lines sigue siendo correcto.
                    visuals.setPrimitiveType(sf::PrimitiveType::Lines);
                    visuals.clear();
                    for (const auto &seg : segments)
                    {
                        visuals.append({seg.start, sf::Color::White});
                        visuals.append({seg.end, sf::Color::White});
                    }

                    std::cout << "Animacion completada!" << std::endl;
                    currentState = AppState::DONE;
                    break;
                }

                case AppState::DONE:
                    break;
            }

            animationClock.restart();
        }

        window.clear(sf::Color(20, 20, 80));
        window.draw(visuals);
        window.display();
    }

    return 0;
}
