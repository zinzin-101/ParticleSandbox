#include <iostream>
#include <SFML/Graphics.hpp>
#include "solver.hpp"
#include "renderer.hpp"
#include "utils/number_generator.hpp"
#include "utils/math.hpp"


static sf::Color getRainbow(float t)
{
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * Math::PI);
    const float b = sin(t + 0.66f * 2.0f * Math::PI);
    return {static_cast<uint8_t>(255.0f * r * r),
            static_cast<uint8_t>(255.0f * g * g),
            static_cast<uint8_t>(255.0f * b * b)};
}

static sf::Color getPinkBlue(float t)
{
    // blue: 5, 250, 246
    // pink: 255, 5, 213
    const float r = 255 + (5 - 255)*t;
    const float g = 5 + (250 - 5) * t;
    const float b = 213 + (246 - 213) * t;
    return { static_cast<uint8_t>(r),
            static_cast<uint8_t>(g),
            static_cast<uint8_t>(b) };
}


int32_t main(int32_t, char*[])
{
    // Create window
    constexpr int32_t window_width  = 1000;
    constexpr int32_t window_height = 1000;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Simulation", sf::Style::Default, settings);
    const uint32_t frame_rate = 60;
    window.setFramerateLimit(frame_rate);

    Solver   solver;
    Renderer renderer{window};

    // Solver configuration
    solver.setConstraint({static_cast<float>(window_width) * 0.5f, static_cast<float>(window_height) * 0.5f}, 450.0f);
    solver.setSubStepsCount(8);
    solver.setSimulationUpdateRate(frame_rate);

    // Set simulation attributes
    const float        object_spawn_delay    = 0.02f;
    const float        object_spawn_speed    = 1000.0f;
    const sf::Vector2f object_spawn_position = {80.0f, 150.0f};
    const float        object_min_radius     = 5.0f;
    const float        object_max_radius     = 8.0f;
    const uint32_t     max_objects_count     = 2500;
    const float        max_angle             = 1.0f;
    float colorIndex = 0;

    // setup font
    sf::Font font;
    if (!font.loadFromFile("THSarabunNew.ttf")){/* error...*/ }
    sf::Text text;
    text.setFont(font); // font is a sf::Font
    text.setString(L"��к�� + ��͡");
    text.setCharacterSize(100); // in pixels, not points!
    text.setFillColor(sf::Color::White);
    text.setPosition(480, 50);

    // create rope
    int objIndex = 0;
    sf::Vector2f startRopePos = {300,600};
    solver.addObject(startRopePos, 5, true);
    solver.setObjectVelocity(solver.m_objects[objIndex], sf::Vector2f{0,0});
    solver.m_objects[objIndex].color = sf::Color::White;//getPinkBlue(objIndex/30.0f);
    objIndex++;
    for (int i = 1; i < 29; i++) {
        solver.addObject(startRopePos + sf::Vector2f(i*15,0), 5, false);
        solver.setObjectVelocity(solver.m_objects[objIndex], sf::Vector2f{ 0,0 });
        solver.m_objects[objIndex].color = sf::Color::White;// getPinkBlue(objIndex / 30.0f);
        Link& link1 = solver.addLink(objIndex - 1, objIndex);
        objIndex++;
    }
    solver.addObject(startRopePos + sf::Vector2f(29 * 15, 0), 5, true);
    solver.setObjectVelocity(solver.m_objects[objIndex], sf::Vector2f{ 0,0 });
    solver.m_objects[objIndex].color = sf::Color::White;//getPinkBlue(objIndex / 30.0f);
    Link& link1 = solver.addLink(objIndex - 1, objIndex);
    objIndex++;


    sf::Clock clock;
    // Main loop
    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        }

        if (solver.getObjectsCount() < max_objects_count && clock.getElapsedTime().asSeconds() >= object_spawn_delay) {
            clock.restart();
            for (int i = 0; i < 5; i++) {
                VerletObject& object = solver.addObject(object_spawn_position + sf::Vector2f(0,i* object_max_radius*2), RNGf::getRange(object_min_radius, object_max_radius), false);
                const float angle = 0.5f;//max_angle * sin(t) + Math::PI * 0.5f;
                solver.setObjectVelocity(object, object_spawn_speed * sf::Vector2f{ cos(angle), sin(angle) });
                object.color = getPinkBlue(colorIndex);
                std::cout << solver.getObjectsCount() << std::endl;
            }
            // inc color
            colorIndex+=0.005f;
            if (colorIndex >= 1.0f) colorIndex = 0.0f;
        }

        solver.update();
        window.clear(sf::Color::White);
        renderer.render(solver);

        window.draw(text);
		window.display();
    }

    return 0;
}
