//
// Created by juan-diego on 3/29/24.
//

#ifndef HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
#define HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H


#include "window_manager.h"
#include "graph.h"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <limits>
#include <cmath>

// Este enum sirve para identificar el algoritmo que el usuario desea simular
enum Algorithm {
    None,
    Dijkstra,
    BestFirst,
    AStar
};


//* --- PathFindingManager ---
//
// Esta clase sirve para realizar las simulaciones de nuestro grafo.
//
// Variables miembro
//     - path           : Contiene el camino resultante del algoritmo que se desea simular
//     - visited_edges  : Contiene todas las aristas que se visitaron en el algoritmo, notar que 'path'
//                        es un subconjunto de 'visited_edges'.
//     - window_manager : Instancia del manejador de ventana, es utilizado para dibujar cada paso del algoritmo
//     - src            : Nodo incial del que se parte en el algoritmo seleccionado
//     - dest           : Nodo al que se quiere llegar desde 'src'
//*
class PathFindingManager {
    WindowManager *window_manager;
    std::vector<sfLine> path;
    std::vector<sfLine> visited_edges;

    struct Entry {
        Node *node;
        double dist;

        // Para usarlo en std::set como priority queue (orden por distancia, y rompe empates por puntero)
        bool operator<(const Entry &other) const {
            if (dist == other.dist) {
                return node < other.node;
            }
            return dist < other.dist;
        }
    };

    // Heurística: distancia euclidiana entre dos nodos (en la proyección de la ventana)
    double heuristic(Node *a, Node *b) const {
        double dx = static_cast<double>(a->coord.x - b->coord.x);
        double dy = static_cast<double>(a->coord.y - b->coord.y);
        return std::sqrt(dx * dx + dy * dy);
    }

    // --- Dijkstra ---
    // Minimiza el costo total desde src hasta todos los nodos (usamos length como peso de las aristas).
    void dijkstra(Graph &graph) {
        std::unordered_map<Node *, double> dist;
        std::unordered_map<Node *, Node *> parent;
        std::set<Entry> open;

        // Inicializar distancias
        for (auto &p : graph.nodes) {
            Node *node = p.second;
            dist[node] = std::numeric_limits<double>::infinity();
        }
        dist[src] = 0.0;
        parent[src] = nullptr;

        open.insert({src, 0.0});

        while (!open.empty()) {
            Entry current = *open.begin();
            open.erase(open.begin());
            Node *u = current.node;

            // Entrada obsoleta (tenemos una mejor distancia registrada)
            if (current.dist > dist[u]) {
                continue;
            }

            if (u == dest) {
                break;
            }

            // Relajación de aristas
            for (Edge *edge : u->edges) {
                Node *v = (edge->src == u) ? edge->dest : edge->src;
                double weight = edge->length; // costo de la arista
                double new_dist = dist[u] + weight;

                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    parent[v] = u;
                    open.insert({v, new_dist});
                }

                // Registrar arista visitada (para el modo "E" de extra lines)
                visited_edges.emplace_back(u->coord, v->coord,
                                           sf::Color(100, 100, 255), 1.0f);
            }

            //render(graph);
        }

        set_final_path(parent);
    }

    // --- Best First Search (Greedy) ---
    // Siempre expande el nodo con menor heurística h(n) (distancia recta al destino).
    void best_first_search(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        std::set<Entry> open;
        std::unordered_set<Node *> closed;
        std::unordered_set<Node *> in_open;

        parent[src] = nullptr;
        open.insert({src, heuristic(src, dest)});
        in_open.insert(src);

        while (!open.empty()) {
            Entry current = *open.begin();
            open.erase(open.begin());
            Node *u = current.node;
            in_open.erase(u);

            if (u == dest) {
                break;
            }

            closed.insert(u);

            for (Edge *edge : u->edges) {
                Node *v = (edge->src == u) ? edge->dest : edge->src;

                if (closed.count(v)) {
                    continue;
                }

                // Sólo insertamos si aún no está en la frontera
                if (!in_open.count(v)) {
                    parent[v] = u;
                    double h = heuristic(v, dest);
                    open.insert({v, h});
                    in_open.insert(v);
                }

                visited_edges.emplace_back(u->coord, v->coord,
                                           sf::Color(100, 255, 100), 1.0f);
            }

            //render(graph);
        }

        set_final_path(parent);
    }

    // --- A* ---
    // Usa f(n) = g(n) + h(n), donde g(n) es el costo acumulado y h(n) la heurística.
    void a_star(Graph &graph) {
        std::unordered_map<Node *, double> g;
        std::unordered_map<Node *, Node *> parent;
        std::set<Entry> open;

        for (auto &p : graph.nodes) {
            Node *node = p.second;
            g[node] = std::numeric_limits<double>::infinity();
        }
        g[src] = 0.0;
        parent[src] = nullptr;

        open.insert({src, heuristic(src, dest)});

        while (!open.empty()) {
            Entry current = *open.begin();
            open.erase(open.begin());
            Node *u = current.node;

            double f_now = g[u] + heuristic(u, dest);
            if (current.dist > f_now) {
                continue; // entrada obsoleta
            }

            if (u == dest) {
                break;
            }

            for (Edge *edge : u->edges) {
                Node *v = (edge->src == u) ? edge->dest : edge->src;
                double weight = edge->length;
                double tentative_g = g[u] + weight;

                if (tentative_g < g[v]) {
                    g[v] = tentative_g;
                    parent[v] = u;
                    double f = tentative_g + heuristic(v, dest);
                    open.insert({v, f});
                }

                visited_edges.emplace_back(u->coord, v->coord,
                                           sf::Color(255, 165, 0), 1.0f);
            }

            //render(graph);
        }

        set_final_path(parent);
    }

    //* --- render ---
    // En cada iteración de los algoritmos esta función es llamada para dibujar los cambios en el 'window_manager'
    void render(Graph &graph) {
        sf::sleep(sf::milliseconds(10));

        window_manager->clear();
        graph.draw();
        // Dibujamos las aristas visitadas y, si ya hay, el path parcial
        draw(true);
        window_manager->display();
    }

    //* --- set_final_path ---
    // Esta función se usa para asignarle un valor a 'this->path' al final de la simulación del algoritmo.
    //
    // 'parent' es un std::unordered_map que recibe un puntero a un vértice y devuelve el vértice anterior a el,
    // formando así el 'path'.
    //
    // Luego, this->path = [Line(a.coord, b.coord), Line(b.coord, c.coord), ...]
    //
    // Este path será utilizado para hacer el 'draw()' del 'path' entre 'src' y 'dest'.
    //*
    void set_final_path(std::unordered_map<Node *, Node *> &parent) {
        path.clear();

        if (src == nullptr || dest == nullptr) {
            return;
        }
        if (src == dest) {
            return;
        }

        // Si dest no aparece en parent, no se encontró camino
        if (parent.find(dest) == parent.end()) {
            return;
        }

        Node *current = dest;
        while (current != nullptr && current != src) {
            Node *prev = parent[current];
            if (prev == nullptr) {
                break;
            }

            // Arista del camino final (dibujada en rojo y más gruesa)
            path.emplace_back(prev->coord, current->coord,
                              sf::Color::Red, 2.5f);

            current = prev;
        }
    }

public:
    Node *src = nullptr;
    Node *dest = nullptr;

    explicit PathFindingManager(WindowManager *window_manager)
            : window_manager(window_manager) {}

    void exec(Graph &graph, Algorithm algorithm) {
        if (src == nullptr || dest == nullptr) {
            return;
        }

        // Limpiar simulación previa
        path.clear();
        visited_edges.clear();

        // Restaurar todos los nodos a sus valores por defecto
        for (auto &p : graph.nodes) {
            p.second->reset();
        }

        // Volver a marcar src y dest con colores especiales
        src->color = sf::Color::Green;
        src->radius = 3.0f;
        dest->color = sf::Color::Cyan;
        dest->radius = 3.0f;

        switch (algorithm) {
            case Dijkstra:
                dijkstra(graph);
                break;
            case BestFirst:
                best_first_search(graph);
                break;
            case AStar:
                a_star(graph);
                break;
            default:
                break;
        }
    }

    void reset() {
        path.clear();
        visited_edges.clear();

        if (src) {
            src->reset();
            src = nullptr;
            // ^^^ Pierde la referencia luego de restaurarlo a sus valores por defecto
        }
        if (dest) {
            dest->reset();
            dest = nullptr;
            // ^^^ Pierde la referencia luego de restaurarlo a sus valores por defecto
        }
    }

    void draw(bool draw_extra_lines) {
        // Dibujar todas las aristas visitadas
        if (draw_extra_lines) {
            for (sfLine &line: visited_edges) {
                line.draw(window_manager->get_window(), sf::RenderStates::Default);
            }
        }

        // Dibujar el camino resultante entre 'src' y 'dest'
        for (sfLine &line: path) {
            line.draw(window_manager->get_window(), sf::RenderStates::Default);
        }

        // Dibujar el nodo inicial
        if (src != nullptr) {
            src->draw(window_manager->get_window());
        }

        // Dibujar el nodo final
        if (dest != nullptr) {
            dest->draw(window_manager->get_window());
        }
    }
};


#endif //HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
