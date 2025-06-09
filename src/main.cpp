#include <omp.h>

#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include "CaveGenerator.hpp"
#include "MarchingSquares.hpp"

const int GLOBAL_MAP_WIDTH = 160;
const int GLOBAL_MAP_HEIGHT = 120;
const float TILE_SIZE_PX = 6.0f;
const int SMOOTHING_ITERATIONS = 4;
const float INITIAL_FILL_PROBABILITY = 0.45f;

int main(int argc, char *argv[])
{
    std::cout << "CavaParallelisOMP" << std::endl;
    std::cout << "Usando OpenMP. Mapa Global: " << GLOBAL_MAP_WIDTH << "x" << GLOBAL_MAP_HEIGHT
              << std::endl;
#pragma omp parallel
    {
#pragma omp single
        {
            std::cout << "Numero de hilos OpenMP: " << omp_get_num_threads() << std::endl;
        }
    }

    if (GLOBAL_MAP_HEIGHT <= 0 || GLOBAL_MAP_WIDTH <= 0)
    {
        std::cout << "Mapa global vacío, nada que hacer." << std::endl;
        return 0;
    }

    unsigned int global_seed =
            static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());

    CaveGenerator caveGen(GLOBAL_MAP_WIDTH, GLOBAL_MAP_HEIGHT, global_seed);

    caveGen.initializeMap(INITIAL_FILL_PROBABILITY);
    std::cout << "Mapa inicial generado." << std::endl;

    std::cout << "Iniciando " << SMOOTHING_ITERATIONS << " iteraciones de suavizado..."
              << std::endl;
    for (int iter = 0; iter < SMOOTHING_ITERATIONS; ++iter)
    {
        caveGen.smoothMapIteration();
    }
    std::cout << "Suavizado completado." << std::endl;

    MarchingSquares msAlgo(TILE_SIZE_PX);
    std::cout << "Generando malla con Marching Squares..." << std::endl;

    std::vector<LineSegment> allSegments = msAlgo.generateMesh(caveGen.getLocalGrid());
    std::cout << "Malla generada. Total segmentos: " << allSegments.size() << std::endl;

    sf::RenderWindow window(sf::VideoMode({(unsigned int) (GLOBAL_MAP_WIDTH * TILE_SIZE_PX),
                                           (unsigned int) (GLOBAL_MAP_HEIGHT * TILE_SIZE_PX)}),
                            "CavaParallelisOMP (Memoria Compartida)");
    window.setFramerateLimit(60);

    sf::VertexArray lines(sf::PrimitiveType::Lines);
    for (const auto &seg : allSegments)
    {
        lines.append({seg.start, sf::Color::White});
        lines.append({seg.end, sf::Color::White});
    }
    std::cout << "VertexArray tiene " << lines.getVertexCount() << " vértices." << std::endl;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                break;
            }
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Q)
                {
                    window.close();
                }
            }
        }
        window.clear(sf::Color(20, 20, 80));
        window.draw(lines);
        window.display();
    }

    return 0;
}
