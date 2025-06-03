#include <mpi.h>

#include <SFML/Graphics.hpp>
#include <algorithm>  // Para std::min, std::max
#include <chrono>     // Para la semilla
#include <iostream>
#include <numeric>  // Para std::iota
#include <vector>

#include "CaveGenerator.hpp"
#include "MarchingSquares.hpp"

// --- Constantes Globales del Mapa ---
const int GLOBAL_MAP_WIDTH = 160;   // Ancho del mapa completo en "tiles"
const int GLOBAL_MAP_HEIGHT = 120;  // Alto del mapa completo en "tiles"
const float TILE_SIZE_PX = 6.0f;    // Tamaño de cada tile en píxeles para SFML
const int SMOOTHING_ITERATIONS = 5;
const float INITIAL_FILL_PROBABILITY = 0.45f;

// --- Constantes para MPI y Serialización (igual que antes) ---
const int FLOATS_PER_SEGMENT = 4;
std::vector<float> serializeSegments(const std::vector<LineSegment> &segments)
{
    std::vector<float> flat_data;
    flat_data.reserve(segments.size() * FLOATS_PER_SEGMENT);
    for (const auto &seg : segments)
    {
        flat_data.push_back(seg.start.x);
        flat_data.push_back(seg.start.y);
        flat_data.push_back(seg.end.x);
        flat_data.push_back(seg.end.y);
    }
    return flat_data;
}
std::vector<LineSegment> deserializeSegments(const float *data, int num_segments)
{
    std::vector<LineSegment> segments;
    segments.reserve(num_segments);
    for (int i = 0; i < num_segments; ++i)
    {
        LineSegment seg;
        seg.start.x = data[i * FLOATS_PER_SEGMENT + 0];
        seg.start.y = data[i * FLOATS_PER_SEGMENT + 1];
        seg.end.x = data[i * FLOATS_PER_SEGMENT + 2];
        seg.end.y = data[i * FLOATS_PER_SEGMENT + 3];
        segments.push_back(seg);
    }
    return segments;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // --- Distribución de trabajo ---
    // Cada proceso maneja un bloque de filas contiguas
    int rows_per_process_base = GLOBAL_MAP_HEIGHT / world_size;
    int remainder_rows = GLOBAL_MAP_HEIGHT % world_size;

    int my_local_height = rows_per_process_base + (world_rank < remainder_rows ? 1 : 0);
    int my_global_start_row =
            world_rank * rows_per_process_base + std::min(world_rank, remainder_rows);

    if (world_rank == 0)
    {
        std::cout << "CavaParallelisMPI" << std::endl;
        std::cout << "Procesos: " << world_size << ", Mapa Global: " << GLOBAL_MAP_WIDTH << "x"
                  << GLOBAL_MAP_HEIGHT << std::endl;
        for (int i = 0; i < world_size; ++i)
        {
            int h = rows_per_process_base + (i < remainder_rows ? 1 : 0);
            int s = i * rows_per_process_base + std::min(i, remainder_rows);
            std::cout << "  Rank " << i << ": " << h << " filas, desde global " << s << std::endl;
        }
    }
    if (my_local_height <= 0 && GLOBAL_MAP_HEIGHT > 0)
    {  // No hay trabajo para este proceso si el mapa es muy pequeño
        std::cerr << "Rank " << world_rank
                  << " no tiene filas asignadas. El mapa es demasiado pequeño para " << world_size
                  << " procesos." << std::endl;
        MPI_Finalize();
        return 1;
    }
    if (GLOBAL_MAP_HEIGHT == 0)
    {  // Mapa vacío
        if (world_rank == 0)
            std::cout << "Mapa global vacío, nada que hacer." << std::endl;
        MPI_Finalize();
        return 0;
    }

    // --- Semilla Global (igual para todos los procesos) ---
    unsigned int global_seed =
            static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
    // Transmitir la semilla desde el rank 0 a todos los demás para consistencia
    MPI_Bcast(&global_seed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    // --- Fase 1: Generación de Mapa Inicial Distribuida ---
    CaveGenerator caveGen(GLOBAL_MAP_WIDTH, my_local_height, global_seed, my_global_start_row);
    if (my_local_height > 0)
    {  // Solo si este proceso tiene filas que generar
        caveGen.initializeMap(INITIAL_FILL_PROBABILITY);
    }
    MPI_Barrier(MPI_COMM_WORLD);  // Esperar a que todos terminen la inicialización

    // --- Fase 2: Suavizado con Halo Exchange ---
    GridRow top_halo_recv_buffer(GLOBAL_MAP_WIDTH);
    GridRow bottom_halo_recv_buffer(GLOBAL_MAP_WIDTH);

    int prev_rank = (world_rank == 0) ? MPI_PROC_NULL : world_rank - 1;
    int next_rank = (world_rank == world_size - 1) ? MPI_PROC_NULL : world_rank + 1;

    for (int iter = 0; iter < SMOOTHING_ITERATIONS; ++iter)
    {
        if (my_local_height == 0)
            continue;  // Si no tengo filas, no participo

        GridRow my_first_row_to_send = caveGen.getFirstRowData();
        GridRow my_last_row_to_send = caveGen.getLastRowData();

        // Intercambiar halos:
        // Enviar mi ÚLTIMA fila al SIGUIENTE proceso (él la recibe como su halo SUPERIOR)
        // Recibir del proceso ANTERIOR su ÚLTIMA fila (yo la recibo como mi halo SUPERIOR)
        MPI_Sendrecv(my_last_row_to_send.data(), GLOBAL_MAP_WIDTH, MPI_INT, next_rank, 0,   // Tag 0
                     top_halo_recv_buffer.data(), GLOBAL_MAP_WIDTH, MPI_INT, prev_rank, 0,  // Tag 0
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Enviar mi PRIMERA fila al proceso ANTERIOR (él la recibe como su halo INFERIOR)
        // Recibir del SIGUIENTE proceso su PRIMERA fila (yo la recibo como mi halo INFERIOR)
        MPI_Sendrecv(my_first_row_to_send.data(), GLOBAL_MAP_WIDTH, MPI_INT, prev_rank, 1,  // Tag 1
                     bottom_halo_recv_buffer.data(), GLOBAL_MAP_WIDTH, MPI_INT, next_rank,
                     1,  // Tag 1
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Si prev_rank o next_rank es MPI_PROC_NULL, Sendrecv no hace nada para esa parte.
        // CaveGenerator::smoothMapIteration debe manejar halos vacíos (considerarlos paredes)
        GridRow &actual_top_halo = (prev_rank == MPI_PROC_NULL) ? GridRow() : top_halo_recv_buffer;
        GridRow &actual_bottom_halo =
                (next_rank == MPI_PROC_NULL) ? GridRow() : bottom_halo_recv_buffer;

        caveGen.smoothMapIteration(actual_top_halo, actual_bottom_halo);
        MPI_Barrier(MPI_COMM_WORLD);  // Sincronizar antes de la siguiente iteración de suavizado
    }

    // --- Fase 3: Preparación para Marching Squares (Necesita una fila extra si no es el último
    // proceso) ---
    MarchingSquares msAlgo(TILE_SIZE_PX);
    std::vector<LineSegment> localSegments;
    LocalGrid grid_for_ms = caveGen.getLocalGrid();  // Copia de la grid local suavizada

    if (my_local_height > 0)
    {  // Solo si tengo filas
        if (next_rank != MPI_PROC_NULL)
        {  // Si no soy el último, necesito una fila de mi vecino 'next'
            GridRow bottom_halo_for_ms(GLOBAL_MAP_WIDTH);
            GridRow dummy_send_buffer;  // No necesito enviar nada aquí, solo recibir

            // El proceso `next_rank` me envía su PRIMERA fila.
            // Yo, `world_rank`, la recibo como mi `bottom_halo_for_ms`.
            // Podríamos usar Isend/Irecv o un Sendrecv más simple.
            // Para simplificar, el `next_rank` envía y yo recibo.
            if (world_rank == next_rank - 1)
            {  // Soy el que está antes de next_rank
                MPI_Recv(bottom_halo_for_ms.data(), GLOBAL_MAP_WIDTH, MPI_INT, next_rank, 2,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                grid_for_ms.push_back(bottom_halo_for_ms);  // Añadir como última fila
            }
            if (world_rank == prev_rank + 1 && prev_rank != MPI_PROC_NULL)
            {  // Soy el next_rank de alguien
                GridRow my_first_row = caveGen.getFirstRowData();
                MPI_Send(my_first_row.data(), GLOBAL_MAP_WIDTH, MPI_INT, prev_rank, 2,
                         MPI_COMM_WORLD);
            }
        }
        // La compensación global Y para los segmentos es my_global_start_row
        localSegments = msAlgo.generateMesh(grid_for_ms, my_global_start_row);
    }
    MPI_Barrier(MPI_COMM_WORLD);  // Asegurarse de que todos los sends/recvs para MS se completen

    // --- Fase 4: Recopilación de resultados en el proceso 0 (igual que antes) ---
    std::vector<float> flatLocalSegments = serializeSegments(localSegments);
    int local_segment_count = localSegments.size();

    std::vector<int> segment_counts(world_rank == 0 ? world_size : 0);
    MPI_Gather(&local_segment_count, 1, MPI_INT, segment_counts.data(), 1, MPI_INT, 0,
               MPI_COMM_WORLD);

    std::vector<int> recv_counts_float(world_rank == 0 ? world_size : 0);
    std::vector<int> displacements_float(world_rank == 0 ? world_size : 0);
    std::vector<float> all_flat_segments_data;
    int total_flat_segments_data_size = 0;

    if (world_rank == 0)
    {
        for (int i = 0; i < world_size; ++i)
        {
            recv_counts_float[i] = segment_counts[i] * FLOATS_PER_SEGMENT;
            displacements_float[i] =
                    (i == 0) ? 0 : displacements_float[i - 1] + recv_counts_float[i - 1];
            total_flat_segments_data_size += recv_counts_float[i];
        }
        all_flat_segments_data.resize(total_flat_segments_data_size);
    }

    MPI_Gatherv(flatLocalSegments.data(), local_segment_count * FLOATS_PER_SEGMENT, MPI_FLOAT,
                all_flat_segments_data.data(), recv_counts_float.data(), displacements_float.data(),
                MPI_FLOAT, 0, MPI_COMM_WORLD);

    // --- Fase 5: Renderizado en el proceso 0 con SFML (igual que antes) ---
    if (world_rank == 0)
    {
        std::cout << "Proceso 0: Recopilación completa. Total floats: "
                  << total_flat_segments_data_size << std::endl;
        std::vector<LineSegment> allSegments;
        if (total_flat_segments_data_size > 0)
        {
            int current_offset_idx = 0;
            for (int i = 0; i < world_size; ++i)
            {
                if (segment_counts[i] > 0)
                {
                    std::vector<LineSegment> proc_segments = deserializeSegments(
                            all_flat_segments_data.data() + displacements_float[i],
                            segment_counts[i]);
                    allSegments.insert(allSegments.end(), proc_segments.begin(),
                                       proc_segments.end());
                }
            }
        }
        std::cout << "Proceso 0: Total segmentos des-serializados: " << allSegments.size()
                  << std::endl;

        sf::RenderWindow window(sf::VideoMode({(unsigned int) (GLOBAL_MAP_WIDTH * TILE_SIZE_PX),
                                               (unsigned int) (GLOBAL_MAP_HEIGHT * TILE_SIZE_PX)}),
                                "CavaParallelisMPI (Distribuido)");
        window.setFramerateLimit(60);

        sf::VertexArray lines(sf::Lines);
        for (const auto &seg : allSegments)
        {
            lines.append(sf::Vertex(seg.start, sf::Color::White));
            lines.append(sf::Vertex(seg.end, sf::Color::White));
        }
        std::cout << "Proceso 0: VertexArray tiene " << lines.getVertexCount() << " vértices."
                  << std::endl;

        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
            }
            window.clear(sf::Color(20, 20, 80));  // Fondo azul oscuro
            window.draw(lines);
            window.display();
        }
    }

    MPI_Finalize();
    return 0;
}
