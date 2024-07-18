#pragma once
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <stdlib.h>

#include "utils/math.hpp"

extern int points[MAXPOINTS][2];
extern int diff_points[MAXPOINTS][2];

enum TYPE {
    SAND,
    WATER,
    CONCRETE,
    LAVA,
    GAS,
    OBSIDIAN,
    NONE
};

sf::Vector2i currentMousePos;
sf::Vector2i lastMousePos;

struct VerletObject
{
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float        mass          = 1.0f;
    float        bounce        = 0.0f;
    float        radius        = 10.0f;
    sf::Color    color         = sf::Color::White;
    bool         pinned        = false;
    TYPE         type          = NONE;
    float        frictionCoeff = 1.0f;
    bool         grounded = false;

    VerletObject() = default;
    VerletObject(sf::Vector2f position_, float radius_, bool pin_, TYPE type_)
        : position{position_}
        , position_last{position_}
        , acceleration{0.0f, 0.0f}
        , radius{radius_}
        , pinned{pin_}
        , type{type_}
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

    VerletObject& addObject(sf::Vector2f position, TYPE type)
    {
        VerletObject obj = {position, 1.f, false, type};
        switch (type)
        {
        case SAND:
            obj.color = { 255,255,0 };
            obj.radius = 6.0f;
            obj.pinned = false;
            obj.mass = 1.5f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.1f;
            break;
        case WATER:
            obj.color = { 0,0,255 };
            obj.radius = 3.5f;
            obj.pinned = false;
            obj.mass = 1.0f;
            obj.bounce = 0.0f;
            break;
        case CONCRETE:
            obj.color = { 100,100,100 };
            obj.radius = 10.0f;
            obj.pinned = true;
            obj.mass = 3.0f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.1f;
            break;
        case LAVA:
            obj.color = { 255,69,0 };
            obj.radius = 4.0f;
            obj.pinned = false;
            obj.mass = 1.25f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.7f;
            break;
        case GAS:
            obj.color = { 200,200,200 };
            obj.radius = 1.0f;
            obj.pinned = false;
            obj.mass = 0.1f;
            obj.bounce = 0.0f;
            break;
        case OBSIDIAN:
            obj.color = {50,50,50};
            obj.radius = 4.0f;
            obj.pinned = false;
            obj.mass = 3.0f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.0f;
            break;
        default:
            obj.color = sf::Color::White;
            obj.radius = 1.0f;
            obj.pinned = false;
            obj.mass = 1.0f;
            obj.bounce = 0.0f;
            break;
        }
        return m_objects.emplace_back(obj);
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
            applyGravity();
            applyTouchForce();
            checkCollisions(step_dt);
            applyConstraint(step_dt);
            applyLinkConstraint(step_dt);
            updateObjects(step_dt);
        }

        lastMousePos = currentMousePos;
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

    void updateMousePos(sf::Vector2i mousePos) {
        currentMousePos = mousePos;
    }

    void applyMouseForce() {
        for (VerletObject& obj : m_objects) {
            if (obj.pinned) {
                continue;
            }

            sf::Vector2f targetPos = { (float)currentMousePos.x, (float)currentMousePos.y };
            sf::Vector2f target = targetPos - obj.position;
            sf::Vector2f moveVec = {   (float)currentMousePos.x - (float)lastMousePos.x,
                                        (float)currentMousePos.y - (float)lastMousePos.y 
                                    };
            sf::Vector2f velocityVec = moveVec / getStepDt();
            float distance = sqrt(target.x * target.x + target.y * target.y);
            if (distance < 250) {
                obj.accelerate(velocityVec * 10.0f);
            }
        }
    }

    sf::Vector2i getCurrentMousePos() {
        return currentMousePos;
    }

    void deleteBrush(float radius) {
        for (uint64_t i = 0; i < m_objects.size(); i++) {
            VerletObject& obj = m_objects[i];

            float distance = sqrt((currentMousePos.x - obj.position.x) * (currentMousePos.x - obj.position.x)
                                + (currentMousePos.y - obj.position.y) * (currentMousePos.y - obj.position.y));
            if (distance < radius) {
                m_objects.erase(m_objects.begin() + i--);
            }
        }
    }

    void clearAll() {
        m_objects.clear();
    }

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
            if (!obj.pinned) {
                switch (obj.type)
                {
                case GAS:
                    obj.accelerate(-m_gravity * obj.mass);
                    break;

                default:
                    obj.accelerate(m_gravity * obj.mass);
                }
            }
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
        //const uint64_t objects_count = m_objects.size();
        // Iterate on all objects
        for (uint64_t i{ 0 }; i < m_objects.size(); i++) {
            VerletObject& object_1 = m_objects[i];
            // Iterate on object involved in new collision pairs
            for (uint64_t k{i + 1}; k < m_objects.size(); k++) {
                VerletObject&      object_2 = m_objects[k];

                const sf::Vector2f v        = object_1.position - object_2.position;
                const float        dist2    = v.x * v.x + v.y * v.y;
                const float        min_dist = object_1.radius + object_2.radius;
                // Check overlapping
                if (dist2 < min_dist * min_dist) {
                    const float        dist  = sqrt(dist2);
                    const sf::Vector2f n     = v / dist;
                    /*const float mass_ratio_1 = object_1.radius / (object_1.radius + object_2.radius);
                    const float mass_ratio_2 = object_2.radius / (object_1.radius + object_2.radius);*/
                    const float mass_ratio_1 = object_1.mass / (object_1.mass + object_2.mass);
                    const float mass_ratio_2 = object_2.mass / (object_1.mass + object_2.mass);
                    const float delta        = 0.5f * response_coef * (dist - min_dist);
                    // Update positions

                    bool canUpdate = computeReaction(object_1, object_2, mass_ratio_1, mass_ratio_2, i ,k);

                    if (canUpdate) {
                        if (!object_1.pinned)
                            object_1.position -= n * (mass_ratio_2 * delta);
                        if (!object_2.pinned)
                            object_2.position += n * (mass_ratio_1 * delta);
                    }
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
        //for (auto& obj : m_objects) {
        for (uint64_t i = 0; i < m_objects.size(); i++) {
            auto& obj = m_objects[i];
            /*const sf::Vector2f v    = m_constraint_center - obj.position;
            const float        dist = sqrt(v.x * v.x + v.y * v.y);
            if (dist > (m_constraint_radius - obj.radius)) {
                const sf::Vector2f n = v / dist;
                obj.position = m_constraint_center - n * (m_constraint_radius - obj.radius);
            }*/

            // box constrain
            if (!obj.pinned) {
                float bounce = obj.bounce;

                sf::Vector2 v = obj.position - obj.position_last;
                if (obj.position.x > (1450 - obj.radius)) {
                    obj.position.x = 1450 - obj.radius;
                    //obj.position_last.x = obj.position.x + v.x * bounce;
                }
                if (obj.position.x < (50 + obj.radius)) {
                    obj.position.x = 50 + obj.radius;
                    //obj.position_last.x = obj.position.x + v.x * bounce;
                }
                if (obj.position.y > (950 - obj.radius)) {
                    obj.position.y = 950 - obj.radius;
                    //obj.position_last.y = obj.position.y + v.y * bounce;

                    if (obj.type == OBSIDIAN) {
                        obj.pinned = true;
                    }

                    const float dt = getStepDt();
                    obj.setVelocity(obj.getVelocity(dt) * obj.frictionCoeff, dt);

                    obj.grounded = true;
                }
                if (obj.position.y < (50 + obj.radius)) {

                    if (obj.type == GAS) {
                        m_objects.erase(m_objects.begin() + i--);
                        continue;
                    }

                    obj.position.y = 50 + obj.radius;
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

    float getVectorMagnitude(sf::Vector2f vec) {
        return sqrtf(vec.x * vec.x + vec.y + vec.y);
    }

    bool computeReaction(VerletObject& object_1, VerletObject& object_2, float mass_ratio_1, float mass_ratio_2, uint64_t& i, uint64_t& k) {
        if (object_1.type == GAS && object_2.type == OBSIDIAN || object_1.type == OBSIDIAN && object_2.type == GAS) {
            return false;
        }
        
        if (object_1.type == WATER && object_2.type == LAVA || object_1.type == LAVA && object_2.type == WATER) {
            float midX = (object_1.position.x + object_2.position.x) / 2.0f;
            float midY = (object_1.position.y + object_2.position.y) / 2.0f;

            generateGas(midX, midY);
            addObject({ midX, midY }, OBSIDIAN);

            m_objects.erase(m_objects.begin() + k--);
            m_objects.erase(m_objects.begin() + i--);

            return false;
        }

        if (object_1.type == GAS && object_2.type == WATER || object_1.type == WATER && object_2.type == GAS) {
            if (object_1.type == GAS) {
                object_1.setVelocity({ 0.0f, -200.0f }, getStepDt());
            }

            if (object_2.type == GAS) {
                object_2.setVelocity({ 0.0f, -200.0f }, getStepDt());
            }
        }

        if (object_1.type == OBSIDIAN && object_2.type == OBSIDIAN) {
            if (object_1.grounded || object_2.grounded) {
                object_1.grounded = true;
                object_2.grounded = true;
                object_1.pinned = true;
                object_2.pinned = true;
            }
        }

        if (object_1.frictionCoeff < 0.9f || object_2.frictionCoeff < 0.9f) {
            const float dt = getStepDt();
            if (object_1.frictionCoeff < 0.9f) {
                sf::Vector2f currVel = object_1.getVelocity(dt);
                object_1.setVelocity({ currVel.x * object_1.frictionCoeff, currVel.y }, dt);
            }
            else if (object_2.frictionCoeff < 0.9f) {
                sf::Vector2f currVel = object_2.getVelocity(dt);
                object_2.setVelocity({ currVel.x * object_2.frictionCoeff, currVel.y }, dt);
            }
        }

        /*if (object_1.type == SAND && object_2.type == WATER || object_1.type == WATER && object_2.type == SAND) {
            if (object_1.type == SAND) {
                if (object_1.position.y <= object_2.position.y) {
                    object_1.setVelocity({ 0.0f, 150.0f }, getStepDt());
                }
            }

            if (object_2.type == SAND) {
                if (object_2.position.y <= object_1.position.y) {
                    object_2.setVelocity({ 0.0f, 150.0f }, getStepDt());
                }
            }
        }*/

        float massDiff = abs(object_1.mass - object_2.mass);
        const float dt = getStepDt();
        int randInt = rand() % 2;
        float velX = (randInt == 0 ? 1.0f : -1.0f) * massDiff * 300.0f;
        float velY = abs(velX) * -0.5;
        if (object_1.type != OBSIDIAN && object_2.type != OBSIDIAN) {
            if (object_1.mass > object_2.mass && !object_1.pinned && !object_2.pinned) {
                if (object_1.position.y <= object_2.position.y) {
                    object_2.setVelocity({ velX , velY }, getStepDt());
                    object_1.setVelocity(object_1.getVelocity(dt) * mass_ratio_1, dt);
                    return false;
                }
            }
            else if (object_2.mass > object_1.mass) {
                if (object_2.position.y <= object_1.position.y) {
                    object_1.setVelocity({ velX ,velY }, getStepDt());
                    object_2.setVelocity(object_2.getVelocity(dt) * mass_ratio_2, dt);
                    return false;
                }
            }
        }

        return true;
    }

    void generateGas(float midX, float midY) {
        for (float x = -1.0f; x <= 1.0f; x += 0.5f) {
            for (float y = -1.0f; y <= 1.0f; y += 0.5f) {
                addObject({ midX + x, midY + y }, GAS);
            }
        }
    }
};
