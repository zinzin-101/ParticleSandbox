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
    const float r = 5 + (255 - 5)*t;
    const float g = 250 + (5 - 250) * t;
    const float b = 246 + (213 - 246) * t;
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
    solver.setSubStepsCount(2);
    solver.setSimulationUpdateRate(frame_rate);

    // Set simulation attributes
    const float        object_spawn_delay    = 0.2f;
    const float        object_spawn_speed    = 500.0f;
    const sf::Vector2f object_spawn_position = {100.0f, 300.0f};
    const float        object_min_radius     = 20.0f;
    const float        object_max_radius     = 20.0f;
    const uint32_t     max_objects_count     = 10;
    const float        max_angle             = 1.0f;
    float colorIndex = 0;

    sf::Font font;
    if (!font.loadFromFile("THSarabunNew.ttf"))
    {
        // error...
    }
    sf::Text text;
    text.setFont(font); // font is a sf::Font
    text.setString(L"≈Ÿ°∫Õ≈ x10");
    text.setCharacterSize(100); // in pixels, not points!
    text.setFillColor(sf::Color::White);
    text.setPosition(600, 50);

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
            for (int i = 0; i < 1; i++) {
                auto& object = solver.addObject(object_spawn_position + sf::Vector2f(0,i* object_max_radius*2), RNGf::getRange(object_min_radius, object_max_radius));
                //const float t      = solver.getTime();
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
