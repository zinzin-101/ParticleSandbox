#pragma once
#include "solver.hpp"

extern const int window_width;
extern const int window_height;

class Renderer
{
public:
    explicit
    Renderer(sf::RenderTarget& target)
        : m_target{target}
    {

    }

    void render(const Solver& solver) const
    {
        // Render constraint
        /*const sf::Vector3f constraint = solver.getConstraint();
        sf::CircleShape constraint_background{constraint.z};
        constraint_background.setOrigin(constraint.z, constraint.z);
        constraint_background.setFillColor(sf::Color::Black);
        constraint_background.setPosition(constraint.x, constraint.y);
        constraint_background.setPointCount(128);
        m_target.draw(constraint_background);*/

        sf::RectangleShape rectangle(sf::Vector2f((window_width), (window_height)));
        rectangle.setPosition(0, 0);
        rectangle.setFillColor(sf::Color::Black);
        m_target.draw(rectangle);

        // Render links
        const auto& links = solver.getLinks();
        for (const auto& alink : links) {
            sf::Vertex line[] =
            {
                sf::Vertex(solver.m_objects[alink.obj_1].position),
                sf::Vertex(solver.m_objects[alink.obj_2].position)
            };
            m_target.draw(line, 2, sf::Lines);
        }

        // Render objects
        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        const auto& objects = solver.getObjects();
        for (const auto& obj : objects) {
            circle.setPosition(obj.position);
            circle.setScale(obj.radius, obj.radius);
            circle.setFillColor(obj.color);
            m_target.draw(circle);
        }

        
    }

private:
    sf::RenderTarget& m_target;
};
