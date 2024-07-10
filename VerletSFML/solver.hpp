#pragma once
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

#include "utils/math.hpp"

extern int points[MAXPOINTS][2];
extern int diff_points[MAXPOINTS][2];

extern const int window_width;
extern const int window_height;

extern bool enableGravity;

struct VerletObject
{
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float        radius        = 10.0f;
    sf::Color    color         = sf::Color::White;
    bool         pinned = false;

    VerletObject() = default;
    VerletObject(sf::Vector2f position_, float radius_, bool pin_)
        : position{position_}
        , position_last{position_}
        , acceleration{0.0f, 0.0f}
        , radius{radius_}
        , pinned{pin_}
    {}

    void update(float dt)
    {
        // Compute how much we moved
        const sf::Vector2f displacement = position - position_last;
        // Update position
        position_last = position;
        position      = position + displacement + acceleration * (dt * dt);
        // Reset acceleration
        acceleration  = {};
    }

    void accelerate(sf::Vector2f a)
    {
        acceleration += a;
    }

    void setVelocity(sf::Vector2f v, float dt)
    {
        position_last = position - (v * dt);
    }

    void addVelocity(sf::Vector2f v, float dt)
    {
        position_last -= v * dt;
    }

    [[nodiscard]]
    sf::Vector2f getVelocity(float dt) const
    {
        return (position - position_last) / dt;
    }
};


struct Link
{
    int obj_1;
    int obj_2;
    float target_dist;

    Link() = default;
    Link(int obj1_, int obj2_, float dist)
        : obj_1{ obj1_ }
        , obj_2{ obj2_ }
        , target_dist{ dist }
    {}

};



class Solver
{
public:
    Solver() = default;

    VerletObject& addObject(sf::Vector2f position, float radius, bool pin)
    {
        return m_objects.emplace_back(position, radius, pin);
    }

    Link& addLink(int obj1, int obj2) 
    {
        sf::Vector2 vec12 = m_objects[obj1].position - m_objects[obj2].position;
        float dist = std::sqrt(vec12.x * vec12.x + vec12.y * vec12.y);
        return m_links.emplace_back(obj1, obj2, dist);
    }

    void update()
    {
        m_time += m_frame_dt;
        const float step_dt = getStepDt();
        for (uint32_t i{m_sub_steps}; i--;) {
            if (enableGravity) {
                applyGravity();
            }
            applyTouchForce();
            checkCollisions(step_dt);
            applyConstraint(step_dt);
            applyLinkConstraint(step_dt);
            updateObjects(step_dt);
        }
    }

    void setSimulationUpdateRate(uint32_t rate)
    {
        m_frame_dt = 1.0f / static_cast<float>(rate);
    }

    void setConstraint(sf::Vector2f position, float radius)
    {
        m_constraint_center = position;
        m_constraint_radius = radius;
    }

    void setSubStepsCount(uint32_t sub_steps)
    {
        m_sub_steps = sub_steps;
    }

    void setObjectVelocity(VerletObject& object, sf::Vector2f v)
    {
        object.setVelocity(v, getStepDt());
    }

    [[nodiscard]]
    const std::vector<VerletObject>& getObjects() const
    {
        return m_objects;
    }

    [[nodiscard]]
    const std::vector<Link>& getLinks() const
    {
        return m_links;
    }

    [[nodiscard]]
    sf::Vector3f getConstraint() const
    {
        return {m_constraint_center.x, m_constraint_center.y, m_constraint_radius};
    }

    [[nodiscard]]
    uint64_t getObjectsCount() const
    {
        return m_objects.size();
    }

    [[nodiscard]]
    float getTime() const
    {
        return m_time;
    }

    [[nodiscard]]
    float getStepDt() const
    {
        return m_frame_dt / static_cast<float>(m_sub_steps);
    }

    std::vector<VerletObject> m_objects;
    std::vector<Link>         m_links;

private:
    uint32_t                  m_sub_steps          = 1;
    sf::Vector2f              m_gravity            = {0.0f, 1000.0f};
    sf::Vector2f              m_constraint_center;
    float                     m_constraint_radius  = 100.0f;
    
    float                     m_time               = 0.0f;
    float                     m_frame_dt           = 0.0f;

    void applyGravity()
    {
        for (auto& obj : m_objects) {
            if (!obj.pinned)
                obj.accelerate(m_gravity);
        }
    }

    void applyTouchForce()
    {
        for (auto& obj : m_objects) {
            if (!obj.pinned) {
                for (int i = 0; i < MAXPOINTS; i++) {
                    if (points[i][0] >= 0) {
                        sf::Vector2f touchPoint = {(float) points[i][0], (float) points[i][1] };
                        sf::Vector2f v = obj.position - touchPoint;
                        float dist2 = v.x * v.x + v.y * v.y;
                        float dist = sqrt(dist2);
                        if (dist < 150) {
                            obj.accelerate({ (float)60.0f*diff_points[i][0], (float)60.0f * diff_points[i][1] });
                        }
                    }
                }
            }
        }
    }

    void checkCollisions(float dt)
    {
        const float    response_coef = 0.75f;
        const uint64_t objects_count = m_objects.size();
        // Iterate on all objects
        for (uint64_t i{0}; i < objects_count; ++i) {
            VerletObject& object_1 = m_objects[i];
            // Iterate on object involved in new collision pairs
            for (uint64_t k{i + 1}; k < objects_count; ++k) {
                VerletObject&      object_2 = m_objects[k];
                const sf::Vector2f v        = object_1.position - object_2.position;
                const float        dist2    = v.x * v.x + v.y * v.y;
                const float        min_dist = object_1.radius + object_2.radius;
                // Check overlapping
                if (dist2 < min_dist * min_dist) {
                    const float        dist  = sqrt(dist2);
                    const sf::Vector2f n     = v / dist;
                    const float mass_ratio_1 = object_1.radius / (object_1.radius + object_2.radius);
                    const float mass_ratio_2 = object_2.radius / (object_1.radius + object_2.radius);
                    const float delta        = 0.5f * response_coef * (dist - min_dist);
                    // Update positions
                    if (!object_1.pinned)
                        object_1.position -= n * (mass_ratio_2 * delta);
                    if (!object_2.pinned)
                         object_2.position += n * (mass_ratio_1 * delta);
                }
            }
        }
    }

    void applyLinkConstraint(float dt)
    {
        for (auto& alink : m_links) {
            sf::Vector2 axis = m_objects[alink.obj_1].position - m_objects[alink.obj_2].position;
            float dist = std::sqrt(axis.x * axis.x + axis.y * axis.y);
            sf::Vector2 n = axis / dist;
            float delta = alink.target_dist - dist;
            if (!m_objects[alink.obj_1].pinned)
                m_objects[alink.obj_1].position += 0.5f * delta * n;
            if (!m_objects[alink.obj_2].pinned)
                m_objects[alink.obj_2].position -= 0.5f * delta * n;
        }
    }

    void applyConstraint(float dt)
    {
        for (auto& obj : m_objects) {
            /*const sf::Vector2f v    = m_constraint_center - obj.position;
            const float        dist = sqrt(v.x * v.x + v.y * v.y);
            if (dist > (m_constraint_radius - obj.radius)) {
                const sf::Vector2f n = v / dist;
                obj.position = m_constraint_center - n * (m_constraint_radius - obj.radius);
            }*/

            // box constrain
            if (!obj.pinned) {
                float bounce = 0.8f;
                int border = 50;
                sf::Vector2 v = obj.position - obj.position_last;
                if (obj.position.x > ((window_width - border/2) - obj.radius)) {
                    obj.position.x = (window_width - border / 2) - obj.radius;
                    //obj.position_last.x = obj.position.x + v.x * bounce;
                }
                if (obj.position.x < (obj.radius)) {
                    obj.position.x = obj.radius;
                    //obj.position_last.x = obj.position.x + v.x * bounce;
                }
                if (obj.position.y > ((window_height - border) - obj.radius)) {
                    obj.position.y = (window_height - border) - obj.radius;
                    //obj.position_last.y = obj.position.y + v.y * bounce;
                }
                if (obj.position.y < (obj.radius)) {
                    obj.position.y = obj.radius;
                    //obj.position_last.y = obj.position.y + v.y * bounce;
                }
            }
            

            //std::cout << obj.getVelocity(dt).x << " " << obj.getVelocity(dt).y << std::endl;
        }
    }

    void updateObjects(float dt)
    {
        for (auto& obj : m_objects) {
            if(!obj.pinned)
                obj.update(dt);
        }
    }
};
