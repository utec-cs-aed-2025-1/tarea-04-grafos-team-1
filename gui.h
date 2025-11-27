//
// Created by juan-diego on 3/11/24.
//

#ifndef HOMEWORK_GRAPH_GUI_H
#define HOMEWORK_GRAPH_GUI_H


#include "window_manager.h"
#include "path_finding_manager.h"

#include <cmath>
#include <functional>
#include <map>
#include <limits>

class GUI {
    WindowManager window_manager;
    PathFindingManager path_finding_manager;

    Graph graph;

    // 1NN es un algoritmo muy popular que retorna el 1 Nearest Neighbour (de ahí el nombre 1NN), o vecino más cercano
    // de una coleccion de elementos a una query dada.
    // En este caso, nos interesa conocer cuál es el nodo mas cercano al punto 'query' pasado como parámetro.
    static Node *_1NN(std::map<std::size_t, Node *> &nodes, sf::Vector2i query) {
        Node *nearest = nullptr;
        double min_dist = std::numeric_limits<double>::max();

        std::function<double(sf::Vector2f)> euclidean = [&](sf::Vector2f point) {
            return std::sqrt(
                    std::pow(point.x - static_cast<double>(query.x), 2) +
                    std::pow(point.y - static_cast<double>(query.y), 2)
            );
        };

        for (auto &[_, node]: nodes) {
            double dist = euclidean(node->coord);
            if (dist < min_dist) {
                min_dist = dist;
                nearest = node;
            }
        }

        return nearest;
    }

public:

    explicit GUI(const std::string &nodes_path, const std::string &edges_path)
            : window_manager(), path_finding_manager(&window_manager), graph(&window_manager) {
        // Parsea los nodos y aristas leyéndolos a partir del csv
        graph.parse_csv(nodes_path, edges_path);
        // Para fines de la animación, puede variar dependiendo del computador
        window_manager.get_window().setFramerateLimit(200);
    }

    void main_loop() {
        bool draw_extra_lines = false;

        // Corre la GUI siempre y cuando la ventana esté abierta
        while (window_manager.is_open()) {

            // --- Manejo de eventos (SFML 3) ---
            while (auto event = window_manager.get_window().pollEvent()) {

                // Caso 1: El usuario cerró la ventana
                if (event->is<sf::Event::Closed>()) {
                    window_manager.close();
                    break;
                }

                // Caso 2: El usuario presionó una tecla
                if (auto keyPressed = event->getIf<sf::Event::KeyPressed>()) {

                    switch (keyPressed->code) {
                        // D = Ejecutar Dijkstra
                        case sf::Keyboard::Key::D: {
                            path_finding_manager.exec(graph, Dijkstra);
                            break;
                        }
                        // B = Best First Search
                        case sf::Keyboard::Key::B: {
                            path_finding_manager.exec(graph, BestFirst);
                            break;
                        }
                        // A = Ejecutar A*
                        case sf::Keyboard::Key::A: {
                            path_finding_manager.exec(graph, AStar);
                            break;
                        }
                        // R = limpiar simulación
                        case sf::Keyboard::Key::R: {
                            path_finding_manager.reset();
                            break;
                        }
                        // E = mostrar/ocultar aristas visitadas
                        case sf::Keyboard::Key::E: {
                            draw_extra_lines = !draw_extra_lines;
                            break;
                        }
                        // Q = salir (cerrar ventana)
                        case sf::Keyboard::Key::Q: {
                            window_manager.close();
                            break;
                        }
                        default:
                            break;
                    }

                    continue; // pasar al siguiente evento
                }


                // Caso 3: El usuario presionó el mouse
                if (auto mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                    // En SFML 3 ya tenemos la posición directamente en el evento
                    sf::Vector2i mouse_position = mousePressed->position;

                    // Si no existe un nodo fuente ('src') asignado
                    if (path_finding_manager.src == nullptr) {
                        // Encuentra el vértice más cercano a la posición del mouse y asigna el vértice a 'src'
                        path_finding_manager.src = _1NN(graph.nodes, mouse_position);
                        path_finding_manager.src->color = sf::Color::Green;
                        path_finding_manager.src->radius = 3.0f;
                    }
                    // Si no existe un nodo destino ('dest') asignado
                    else if (path_finding_manager.dest == nullptr) {
                        // Encuentra el vértice más cercano a la posición del mouse y asigna el vértice a 'dest'
                        path_finding_manager.dest = _1NN(graph.nodes, mouse_position);
                        path_finding_manager.dest->color = sf::Color::Cyan;
                        path_finding_manager.dest->radius = 3.0f;
                    }

                    continue;
                }

                // Cualquier otro evento lo ignoramos
            }

            // --- Render (igual que el original) ---

            // Limpia la ventana anterior
            window_manager.clear();

            // Dibuja el grafo en el frame actual
            graph.draw();
            // Dibuja el 'path' resultante de la simulacion,
            // si 'draw_extra_lines' es true, también dibujará el resto de aristas visitadas
            path_finding_manager.draw(draw_extra_lines);

            // Hace un display del frame actual
            window_manager.display();
        }
    }
};


#endif //HOMEWORK_GRAPH_GUI_H
