//
// Created by juan-diego on 3/11/24.
//

#ifndef HOMEWORK_GRAPH_EDGE_H
#define HOMEWORK_GRAPH_EDGE_H

#include <SFML/Graphics.hpp>
#include "node.h"
#include <cstring>
#include <fstream>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include <sstream>

// Color por defecto de todas las aristas (usado por SFML)
sf::Color default_edge_color = sf::Color(255, 200, 100);
// Grosor por defecto de todas las aristas (usado por SFML)
float default_thickness = 0.8;


// No tocar esta clase (solo cambié la primitive type a TriangleStrip para SFML 3)
class sfLine : public sf::Drawable {
public:
    sfLine(const sf::Vector2f& point1, const sf::Vector2f& point2, sf::Color color, float thickness)
        : thickness(thickness) {

        sf::Vector2f direction = point2 - point1;
        float lenSq = direction.x * direction.x + direction.y * direction.y;

        // Si los puntos son iguales (longitud 0), evitamos dividir entre 0.
        if (lenSq == 0.f) {
            for (auto &vertex : Vertices) {
                vertex.position = point1;
                vertex.color = color;
            }
            return;
        }

        sf::Vector2f unitDirection = direction / std::sqrt(lenSq);
        sf::Vector2f unitPerpendicular(-unitDirection.y, unitDirection.x);

        sf::Vector2f offset = (this->thickness / 2.f) * unitPerpendicular;

        Vertices[0].position = point1 + offset;
        Vertices[1].position = point2 + offset;
        Vertices[2].position = point2 - offset;
        Vertices[3].position = point1 - offset;

        for (auto &vertex : Vertices) {
            vertex.color = color;
        }
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
        target.draw(Vertices, 4, sf::PrimitiveType::TriangleStrip);
    }

private:
    sf::Vertex Vertices[4];
    float thickness;
    sf::Color color {};
};


// *
// ---- Edge ----
// Esta estructura contiene la informacion de una arista
//
// Variables miembro
//     - src           : Vértice inicial de la arista
//     - dest          : Vértice final de la arista
//     - max_speed     : Velocidad maxima en la que se puede ir de 'src' a 'dest'
//     - length        : Longitud de la trayectoria de 'src' a 'dest'
//     - one_way       : Si es falso, significa que tambien existe una arista en el grafo de 'dest' a 'src'.
//                       Caso contrario, significa que solo existe la arista actual de 'src' a 'dest'
//     - lanes         : La cantidad de carriles en el camino de 'src' a 'dest'
//     - color         : El color de la linea que conecta a 'src' y 'dest', es usado por SFML
//     - thickness     : El grosor de la linea que conecta a 'src' y 'dest', es usado por SFML
//
// Funciones miembro
//     - parse_csv     : Lee las aristas desde el csv
//     - draw          : Dibuja la arista instanciada
//     - reset         : Setea 'color' y 'thickness' a sus valores por defecto
// *
struct Edge {
    Node *src = nullptr;
    Node *dest = nullptr;
    int max_speed;
    double length;
    bool one_way;
    int lanes;

    sf::Color color = default_edge_color;
    float thickness = default_thickness;

    explicit Edge(Node *src, Node *dest, int max_speed, double length, bool one_way, int lanes)
        : src(src), dest(dest),
          max_speed(max_speed),
          length(length),
          one_way(one_way),
          lanes(lanes) {
    }

    static void
    parse_csv(const std::string &edges_path, std::vector<Edge *> &edges, std::map<std::size_t, Node *> &nodes) {
        edges.reserve(790'509);

        std::ifstream file(edges_path);
        if (!file.is_open()) {
            return;
        }

        std::string line;

        // Leer y descartar cabecera
        if (!std::getline(file, line)) {
            return;
        }

        auto is_number = [](const std::string &s) {
            if (s.empty()) return false;
            for (unsigned char c : s) {
                if (!std::isdigit(c)) return false;
            }
            return true;
        };

        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string src_str, dest_str, max_speed_str, length_str, oneway_str, lanes_str;

            if (!std::getline(ss, src_str, ',')) continue;
            if (!std::getline(ss, dest_str, ',')) continue;
            if (!std::getline(ss, max_speed_str, ',')) continue;
            if (!std::getline(ss, length_str, ',')) continue;
            if (!std::getline(ss, oneway_str, ',')) continue;
            if (!std::getline(ss, lanes_str, ',')) continue;

            // Validar que src y dest sean numéricos
            if (!is_number(src_str) || !is_number(dest_str)) {
                continue;
            }

            try {
                std::size_t src_id = static_cast<std::size_t>(std::stoll(src_str));
                std::size_t dest_id = static_cast<std::size_t>(std::stoll(dest_str));

                auto it_src = nodes.find(src_id);
                auto it_dest = nodes.find(dest_id);
                if (it_src == nodes.end() || it_dest == nodes.end()) {
                    // Arista apunta a un nodo que no existe en el mapa -> ignorar
                    continue;
                }

                int max_speed_val = 0;
                int lanes_val = 0;
                double length_val = 0.0;

                if (!max_speed_str.empty())
                    max_speed_val = std::stoi(max_speed_str);
                if (!lanes_str.empty())
                    lanes_val = std::stoi(lanes_str);
                if (!length_str.empty())
                    length_val = std::stod(length_str);

                bool one_way_val = false;
                if (!oneway_str.empty()) {
                    // En el CSV venía "True"/"False"
                    if (oneway_str == "True" || oneway_str == "true" || oneway_str == "1")
                        one_way_val = true;
                }

                Edge *edge = new Edge(
                        it_src->second,
                        it_dest->second,
                        max_speed_val,
                        length_val,
                        one_way_val,
                        lanes_val
                );
                edges.push_back(edge);

            } catch (const std::exception &) {
                // Cualquier problema de conversión -> ignorar la línea
                continue;
            }
        }
    }

    void draw(sf::RenderWindow &window) const {
        sfLine line(src->coord, dest->coord, color, thickness);
        line.draw(window, sf::RenderStates::Default);
    }
};

#endif //HOMEWORK_GRAPH_EDGE_H
