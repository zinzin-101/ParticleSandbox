#pragma once
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <string>

#include "utils/math.hpp"

#define NUM_OF_TYPE 14

extern int points[MAXPOINTS][2];
extern int diff_points[MAXPOINTS][2];

const enum TYPE {
    SAND,
    WATER,
    CONCRETE,
    LAVA,
    GAS,
    OBSIDIAN,
    DIRT,
    WOOD,
    FIRE,
    FIRE_GAS,
    NONE,
    SPAWNER,
    BLACKHOLE,
    STRING
};

const std::string typeString[NUM_OF_TYPE]{
    "Sand",
    "Water",
    "Concrete",
    "Lava",
    "Gas",
    "Obsidian",
    "Dirt",
    "Wood",
    "Fire",
    "Fire Gas",
    "Force",
    "Spawn",
    "Blackhole",
    "String"
};

float typeRadiusArr[NUM_OF_TYPE] = { 6.0f, 3.5f, 10.0f, 4.0f, 1.0f, 4.0f, 4.0f, 10.0f, 1.5f, 1.0f, 999.0f, 999.0f, 999.0f, 1.0f };

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
    bool         isFluid       = true;
    bool         grounded      = false;
    int          lifespan      = -1;
    int          counter       = -1;
    TYPE         spawnerType   = NONE;

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
            obj.isFluid = false;
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
            obj.isFluid = false;
            break;
        case LAVA:
            obj.color = { 255,69,0 };
            obj.radius = 4.0f;
            obj.pinned = false;
            obj.mass = 1.25f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.8f;
            break;
        case GAS:
            obj.color = { 200,200,200 };
            obj.radius = 1.0f;
            obj.pinned = false;
            obj.mass = 0.1f;
            obj.bounce = 0.0f;
            obj.lifespan = 8;
            break;
        case OBSIDIAN:
            obj.color = {50,50,50};
            obj.radius = 4.0f;
            obj.pinned = false;
            obj.mass = 3.0f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.0f;
            obj.isFluid = false;
            break;
        case DIRT:
            obj.color = { 84, 28, 0 };
            obj.radius = 4.0f;
            obj.pinned = false;
            obj.mass = 1.2f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.1f;
            obj.isFluid = false;
            break;
        case WOOD:
            obj.color = { 227, 159, 64 };
            obj.radius = 10.0f;
            obj.pinned = true;
            obj.mass = 3.0f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.1f;
            obj.isFluid = false;
            break;
        case FIRE:
            obj.color = { 227, 38, 5 };
            obj.radius = 1.5f;
            obj.pinned = false;
            obj.mass = 0.3f;
            obj.bounce = 0.0f;
            obj.frictionCoeff = 0.1f;
            obj.lifespan = 10;
            obj.isFluid = false;
            obj.counter = 1;
            break;
        case FIRE_GAS:
            obj.color = { 150,150,150 };
            obj.radius = 1.0f;
            obj.pinned = false;
            obj.mass = 0.2f;
            obj.bounce = 0.0f;
            obj.lifespan = 8;
            obj.counter = 1;
            break;
        case SPAWNER:
            obj.color = { 101, 2, 158 };
            obj.radius = 5.0f;
            obj.pinned = true;
            break;
        case STRING:
            obj.color = sf::Color::White;
            obj.radius = typeRadiusArr[STRING];
            obj.pinned = false;
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

    void addObjectCluster(sf::Vector2f pos, TYPE type, float size) {
        float halfSize = size / 2.0f;
        float increment = 2.0 * typeRadiusArr[type] - 0.05f;
        for (float i = -halfSize; i <= halfSize; i += increment) {
            for (float j = -halfSize; j <= halfSize; j += increment) {
                sf::Vector2f temp = { i,j };
                addObject(pos + temp, type);
            }
        }
    }

    Link& addLink(int obj1, int obj2) 
    {
        sf::Vector2 vec12 = m_objects[obj1].position - m_objects[obj2].position;
        float dist = std::sqrt(vec12.x * vec12.x + vec12.y * vec12.y);
        return m_links.emplace_back(obj1, obj2, dist);
    }

    void update(bool canUpdate)
    {
        m_time += m_frame_dt;
        const float step_dt = getStepDt();

        if (canUpdate) {
            for (uint32_t i{ m_sub_steps }; i--;) {
                applyGravity();
                //applyTouchForce();
                checkCollisions(step_dt);
                applyConstraint(step_dt);
                applyLinkConstraint(step_dt);
                updateObjects(step_dt);
            }

            updateSpawner();
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

    [[nodiscard]]
    unsigned int getFrameNum() const 
    {
        return m_frame_num;
    }

    std::vector<VerletObject> m_objects;
    std::vector<Link>         m_links;

    void updateMousePos(sf::Vector2i mousePos) {
        currentMousePos = mousePos;
    }

    void updateFrameNum(unsigned int n) {
        m_frame_num = n;
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

    void applyForce(sf::Vector2f currentPos) {
        for (VerletObject& obj : m_objects) {
            if (obj.pinned) {
                continue;
            }
            sf::Vector2f v = currentPos - obj.position;
            float dist2 = v.x * v.x + v.y * v.y;
            float dist = sqrt(dist2);
            if (dist < 150) {
                obj.accelerate(v * 60.0f);
            }
        }
    }

    void applyPushForce(sf::Vector2f currentPos, float radius) {
        for (VerletObject& obj : m_objects) {
            if (obj.pinned) {
                continue;
            }
            sf::Vector2f v = obj.position - currentPos;
            float dist2 = v.x * v.x + v.y * v.y;
            float dist = sqrt(dist2);
            if (dist < radius) {
                //obj.setVelocity({ 0,0 }, getStepDt());
                obj.addVelocity(v * 10.0f * ((radius - dist) / radius), getStepDt());
            }
        }
    }

    void applyCentripetalForce(sf::Vector2f currentPos, float radius, float power) {
        for (VerletObject& obj : m_objects) {
            if (obj.pinned || obj.type == SPAWNER) {
                continue;
            }
            
            sf::Vector2f v = currentPos - obj.position;
            float dist2 = v.x * v.x + v.y * v.y;
            float dist = sqrt(dist2);
            sf::Vector2f n = getNormalizedVector(v);
                        
            //float quadRadius = radius * 4.0f;
            //float doubleRadius = radius * 2.0f;

            sf::Vector2f acceleration = (power * 300.0f * obj.mass / (dist * 5.0f)) * n;
            sf::Vector2f velocity = { n.y, -n.x };
            float a = getVectorMagnitude(acceleration);
            float speed = sqrtf((a * dist) / obj.mass);
            velocity *= speed;

            if (dist < radius * 1.5f) {
                obj.addVelocity(velocity * 2.0f, getStepDt());
                obj.addVelocity((obj.position.y > currentPos.y ? acceleration * 2.0f : acceleration) , getStepDt());
            }
            else if (dist < radius * 5.0f) {
                obj.addVelocity(acceleration * 75.0f, getStepDt());
            }

            /*if (dist < radius) {
                sf::Vector2f oldVel = obj.getVelocity(getStepDt());
                sf::Vector2f newVel = { -oldVel.y, oldVel.x };
                obj.setVelocity({ 0,0 }, getStepDt());
                obj.setVelocity(newVel, getStepDt());

                sf::Vector2f newVec = obj.position - currentPos;
                obj.addVelocity(newVec * 10.0f * ((radius - dist) / radius), getStepDt());
            }*/
        }
    }

    sf::Vector2i getCurrentMousePos() {
        return currentMousePos;
    }

    sf::Vector2f getCurrentMousePosF() {
        sf::Vector2i mousePosInt = getCurrentMousePos();
        sf::Vector2f mousePosF = { (float)mousePosInt.x, (float)mousePosInt.y };
        return mousePosF;
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

    void deleteBrush(float radius, sf::Vector2f pos) {
        for (uint64_t i = 0; i < m_objects.size(); i++) {
            VerletObject& obj = m_objects[i];

            float distance = sqrt((pos.x - obj.position.x) * (pos.x - obj.position.x)
                + (pos.y - obj.position.y) * (pos.y - obj.position.y));
            if (distance < radius) {
                m_objects.erase(m_objects.begin() + i--);
            }
        }
    }

    void deleteObjectsOfType(TYPE type) {
        for (uint64_t i{ 0 }; i < m_objects.size(); i++) {
            VerletObject& obj = m_objects[i];

            if (type == GAS) {
                if (obj.type == FIRE_GAS || obj.type == GAS) {
                    m_objects.erase(m_objects.begin() + i--);
                }
                continue;
            }

            if (obj.type == type) {
                m_objects.erase(m_objects.begin() + i--);

            }
        }
    }

    void deleteSpawnersOfType(TYPE type) {
        for (uint64_t i{ 0 }; i < m_objects.size(); i++) {
            VerletObject& obj = m_objects[i];

            if (obj.spawnerType == type && obj.type == SPAWNER) {
                m_objects.erase(m_objects.begin() + i--);

            }
        }
    }

    void clearHalf() {
        for (uint64_t i{ 0 }; i < m_objects.size(); i++) {
            if (m_objects[i].type == SPAWNER) {
                continue;
            }

            int randNum = rand() % 2;
            if (randNum == 0) {
                m_objects.erase(m_objects.begin() + i--);
            }
        }
    }

    void clearAll() {
        m_objects.clear();
        m_links.clear();
    }

    void deleteBack() {
        m_objects.erase(m_objects.begin() + getObjectsCount() - 1);
    }

    float getVectorMagnitudeSqr(sf::Vector2f vec) {
        return vec.x * vec.x + vec.y * vec.y;
    }

    float getVectorMagnitude(sf::Vector2f vec) {
        return sqrtf(vec.x * vec.x + vec.y * vec.y);
    }

    sf::Vector2f getNormalizedVector(sf::Vector2f v) {
        return v / getVectorMagnitude(v);
    }

    bool readSave(std::string fileName) {
        std::ifstream file(fileName);

        if (!file.is_open()) {
            return false;
        }

        clearAll();

        std::string line;
        while (!file.eof()) {
            if (!std::getline(file, line, ',')) {
                break;
            }
            float x = std::stof(line);

            std::getline(file, line, ',');
            float y = std::stof(line);

            std::getline(file, line, ',');
            int counter = std::stoi(line);

            std::getline(file, line, ',');
            float bounce = std::stof(line);

            std::getline(file, line, ',');
            float friction = std::stof(line);

            std::getline(file, line, ',');
            int type = std::stoi(line);

            std::getline(file, line);
            int spawnerType = std::stoi(line);

            VerletObject& obj = addObject({ x,y }, (TYPE)type);
            obj.spawnerType = (TYPE)spawnerType;
            obj.counter = counter;
            obj.bounce = bounce;
            obj.frictionCoeff = friction;
        }

        file.close();
    }

    bool writeSave(std::string fileName) {
        std::ofstream file(fileName);
        
        if (!file) {
            return false;
        }

        std::ostringstream ss;
        for (VerletObject& obj : m_objects) {
            ss << obj.position.x << "," << obj.position.y << ",";
            ss << obj.counter << ",";
            ss << obj.bounce << ",";
            ss << obj.frictionCoeff << ",";
            ss << (int)obj.type << ",";
            ss << (int)obj.spawnerType << "\n";
        }
        file << ss.str();
        file.close();

        return true;
    }

private:
    uint32_t                  m_sub_steps          = 1;
    sf::Vector2f              m_gravity            = {0.0f, 1000.0f};
    sf::Vector2f              m_constraint_center;
    float                     m_constraint_radius  = 100.0f;
    
    float                     m_time               = 0.0f;
    float                     m_frame_dt           = 0.0f;
    
    unsigned int              m_frame_num          = 0;

    void applyGravity()
    {
        for (auto& obj : m_objects) {
            if (!obj.pinned) {
                switch (obj.type)
                {
                case GAS:
                    obj.accelerate(-m_gravity * obj.mass);
                    break;
                case FIRE_GAS:
                    obj.accelerate(-m_gravity * obj.mass);
                    break;
                default:
                    obj.accelerate(m_gravity * obj.mass);
                    break;
                }
            }
        }
    }

    void applyTouchForce(float power)
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
                            obj.accelerate({ (power * 0.5f) * diff_points[i][0], (power * 0.5f) * diff_points[i][1] });
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

            if (object_1.type == SPAWNER) {
                continue;
            }

            for (uint64_t k{i + 1}; k < m_objects.size(); k++) {
                VerletObject&      object_2 = m_objects[k];

                if (object_2.type == SPAWNER) {
                    continue;
                }

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

                    if (!canUpdate) {
                        continue;
                    }

                    if (!object_1.pinned)
                        object_1.position -= n * (mass_ratio_2 * delta);
                    if (!object_2.pinned)
                        object_2.position += n * (mass_ratio_1 * delta);

                    /*if (!object_1.isFluid && object_1.type != object_2.type)
                        object_1.setVelocity({ 0,0 }, getStepDt());
                    if (!object_2.isFluid && object_1.type != object_2.type)
                        object_2.setVelocity({ 0,0 }, getStepDt());*/
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

                    if (obj.type == GAS || obj.type == FIRE_GAS) {
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
        /*for (auto& obj : m_objects) {
            if(!obj.pinned)
                obj.update(dt);
            passiveBehaviorUpdate(obj);
        }*/

        for (uint64_t i{ 0 }; i < m_objects.size(); i++) {
            auto& obj = m_objects[i];

            if (!obj.pinned)
                obj.update(dt);
            passiveBehaviorUpdate(obj, i);
        }
    }

    void passiveBehaviorUpdate(VerletObject& obj, uint64_t& i) {
        const int frameNum = getFrameNum();
        int randFrame;
        int chance;

        switch (obj.type){
            case GAS:
                if (frameNum % 300 == 0) {
                    obj.lifespan--;

                    if (obj.lifespan == 0) {
                        m_objects.erase(m_objects.begin() + i--);
                    }
                }
                break;

            case FIRE_GAS:
                if (frameNum % 300 == 0) {
                    obj.lifespan--;

                    if (obj.lifespan == 0) {
                        m_objects.erase(m_objects.begin() + i--);
                    }

                    if (obj.counter > 0) {
                        obj.counter--;
                    }
                }
                break;

            case FIRE:
                randFrame = 60 + (rand() % 61);
                chance = 1 + rand() % 1000;

                if (chance > 950 && frameNum % randFrame == 0) {
                    int randX = -50 + (1 + rand() % 100);
                    int randY = -1 * (50 + rand() % 50);

                    VerletObject& tempObj = addObject(obj.position, FIRE_GAS);
                    tempObj.setVelocity({ (float)randX, (float)randY }, getStepDt());
                }

                if (frameNum % 300 == 0) {
                    obj.lifespan--;

                    if (obj.lifespan == 0) {
                        m_objects.erase(m_objects.begin() + i--);
                    }

                    if (obj.counter > 0) {
                        obj.counter--;
                    }
                }
                break;

            case LAVA:
                randFrame = 60 + (rand() % 61);
                chance = 1 + rand() % 1000;

                if (chance > 950 && frameNum % randFrame == 0) {
                    int randX = -150 + (rand() % 301);
                    int randY = -1 * (50 + rand() % 151);

                    VerletObject& tempObj = addObject(obj.position, FIRE);
                    tempObj.setVelocity({ (float)randX, (float)randY }, getStepDt());
                    tempObj.lifespan = 2;
                }
                break;

            case SPAWNER:
                /*if (obj.spawnerType == NONE) {
                    applyPushForce(obj.position, obj.bounce);
                    break;
                }

                if (obj.spawnerType == BLACKHOLE) {
                    applyCentripetalForce(obj.position, obj.bounce, obj.counter);
                    break;
                }

                if (frameNum % obj.counter == 0) {
                    VerletObject& tempObj = addObject(obj.position, obj.spawnerType);
                    int randNum = rand() % 2;
                    float offset = (randNum == 0 ? -0.1f : 0.1f);
                    tempObj.position.x += offset;
                }*/
                break;
        }
    }

    void updateSpawner() {
        const int frameNum = getFrameNum();

        for (VerletObject& obj : m_objects) {
            if (obj.type != SPAWNER) {
                continue;
            }

            if (obj.spawnerType == NONE) {
                applyPushForce(obj.position, obj.bounce);
                continue;
            }

            if (obj.spawnerType == BLACKHOLE) {
                applyCentripetalForce(obj.position, obj.bounce, obj.frictionCoeff);
                continue;
            }

            if (frameNum % obj.counter == 0) {
                VerletObject& tempObj = addObject(obj.position, obj.spawnerType);
                int randNum = rand() % 2;
                float offset = (randNum == 0 ? -0.1f : 0.1f);
                tempObj.position.x += offset;
            }
        }
    }

    bool computeReaction(VerletObject& object_1, VerletObject& object_2, float mass_ratio_1, float mass_ratio_2, uint64_t& i, uint64_t& k) {
        if (object_1.type == GAS && object_2.type == OBSIDIAN || object_1.type == OBSIDIAN && object_2.type == GAS) {
            return false;
        }
        if (object_1.type == GAS && object_2.type == FIRE || object_1.type == FIRE && object_2.type == GAS) {
            return false;
        }
        if (object_1.type == FIRE_GAS && object_2.type == FIRE || object_1.type == FIRE && object_2.type == FIRE_GAS) {
            return false;
        }
        if (object_1.type == FIRE_GAS && object_2.type == GAS || object_1.type == GAS && object_2.type == FIRE_GAS) {
            return false;
        }
        if (object_1.type == FIRE_GAS && object_2.type == FIRE_GAS) {
            return false;
        }
        if (object_1.type == FIRE && object_2.type == LAVA || object_1.type == LAVA && object_2.type == FIRE) {
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

        if (object_1.type == WATER && object_2.type == FIRE || object_1.type == FIRE && object_2.type == WATER) {
            float midX = (object_1.position.x + object_2.position.x) / 2.0f;
            float midY = (object_1.position.y + object_2.position.y) / 2.0f;

            generateGas(midX, midY);

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

        if (object_1.type == WOOD && object_2.type == FIRE || object_1.type == FIRE && object_2.type == WOOD) {
            int randInt = 1 + rand() % 1000;
            if (randInt > 900 && (object_1.counter == 0 || object_2.counter == 0)) {
                sf::Vector2f pos1 = object_1.position;
                sf::Vector2f pos2 = object_2.position;
                if (object_1.type == WOOD) {
                    generateFire(pos1);
                    generateDarkGas(pos1.x, pos1.y);
                }

                if (object_2.type == WOOD) {
                    generateFire(pos2);
                    generateDarkGas(pos2.x, pos2.y);
                }

                m_objects.erase(m_objects.begin() + k--);
                m_objects.erase(m_objects.begin() + i--);

                return false;
            }
        }

        if (object_1.type == WOOD && object_2.type == FIRE_GAS || object_1.type == FIRE_GAS && object_2.type == WOOD) {
            int randInt = 1 + rand() % 1000;
            if (randInt > 980 && (object_1.counter == 0 || object_2.counter == 0)) {
                sf::Vector2f pos1 = object_1.position;
                sf::Vector2f pos2 = object_2.position;
                if (object_1.type == WOOD) {
                    generateFire(pos1);
                    generateDarkGas(pos1.x, pos1.y - object_1.radius * 2.0f);
                }

                if (object_2.type == WOOD) {
                    generateFire(pos2);
                    generateDarkGas(pos2.x, pos2.y - object_2.radius * 2.0f);
                }

                m_objects.erase(m_objects.begin() + k--);
                m_objects.erase(m_objects.begin() + i--);

                return false;
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

        if (object_1.type == OBSIDIAN || object_2.type == OBSIDIAN) {
            if (object_2.pinned && object_1.type == OBSIDIAN) {
                object_1.grounded = true;
                object_1.pinned = true;
            }

            if (object_1.pinned && object_2.type == OBSIDIAN) {
                object_2.grounded = true;
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

            if (object_1.frictionCoeff < 0.9f && object_2.pinned) {
                sf::Vector2f currVel = object_1.getVelocity(dt);
                object_1.setVelocity(currVel * object_1.frictionCoeff, dt);
            }
            else if (object_2.frictionCoeff < 0.9f && object_1.pinned) {
                sf::Vector2f currVel = object_2.getVelocity(dt);
                object_2.setVelocity(currVel * object_2.frictionCoeff, dt);
            }
        }

        float massDiff = abs(object_1.mass - object_2.mass);
        const float dt = getStepDt();
        int randInt = rand() % 2;
        float velX = (randInt == 0 ? 1.0f : -1.0f) * massDiff * 300.0f;
        float velY = abs(velX) * -0.5;
        if (object_1.isFluid || object_2.isFluid) {
            if (object_1.mass > object_2.mass && !object_1.pinned && !object_2.pinned) {
                if (object_1.position.y <= object_2.position.y && object_2.isFluid) {
                    object_2.setVelocity({ velX , velY }, getStepDt());
                    object_1.setVelocity(object_1.getVelocity(dt) * mass_ratio_1, dt);
                    return false;
                }
            }
            else if (object_2.mass > object_1.mass && !object_1.pinned && !object_2.pinned) {
                if (object_2.position.y <= object_1.position.y && object_1.isFluid) {
                    object_1.setVelocity({ velX ,velY }, getStepDt());
                    object_2.setVelocity(object_2.getVelocity(dt) * mass_ratio_2, dt);
                    return false;
                }
            }
        }

        return true;
    }

    void generateGas(float midX, float midY) {
        for (float x = -0.5f; x <= 0.5f; x += 0.5f) {
            for (float y = -0.5f; y <= 0.5f; y += 0.5f) {
                addObject({ midX + x, midY + y }, GAS);
            }
        }
    }

    void generateDarkGas(float midX, float midY) {
        for (float x = -0.5f; x <= 0.5f; x += 0.5f) {
            for (float y = -0.5f; y <= 0.5f; y += 0.5f) {
                VerletObject& obj = addObject({ midX + x, midY + y }, GAS);
                obj.color = { 150,150,150 };
            }
        }
    }

    void generateFire(sf::Vector2f v) {
        for (float x = -2.0f; x <= 2.0f; x += 4.0f) {
            for (float y = -2.0f; y <= 2.0f; y += 4.0f) {
                addObject({ v.x + x, v.y + y }, FIRE);
            }
        }
    }
};
