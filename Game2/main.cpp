#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFML/OpenGL.hpp>
#include <math.h>
#include <vector>

struct Planet
{
    sf::Vector2f position;
    double radius;
    double mass; //MASS IN KG
    sf::Vector2f velocity;
};

long double G = 6.6743e-11;

long double calculateGravityForce(double mass1, double mass2, double distance)
{
        //Force = (G * mass1 * mass2) / distance^2 
        //DISTANCE BEING BETWEEN PLANET CENTERS IN METERS

    long double force = (G * mass1 * mass2) / (distance * distance);

    return force;
}

sf::Vector2f vectorFromPlanets(Planet planet1, Planet planet2)
{
    return planet2.position - planet1.position;
}

sf::Vector2f getVectorFromForce(double mass, long double force, sf::Vector2f direction)
{
    sf::Vector2f newVector;
    double magnitude = (force / mass);
    return direction * static_cast<float>(magnitude);
}

inline float dot(const sf::Vector2f& a, const sf::Vector2f& b) { return a.x * b.x + a.y * b.y; }

void doCircleCollision(Planet planet1, Planet planet2, float restitution = 0.8f)
{
    sf::Vector2f vectorBetweenPlanets = vectorFromPlanets(planet1, planet2);
    float centerDistance = std::sqrt((vectorBetweenPlanets.x * vectorBetweenPlanets.x) + (vectorBetweenPlanets.y * vectorBetweenPlanets.y));
    float minimumDistance = planet1.radius + planet2.radius;


}

constexpr double softening2 = 1e6;         // (meters^2) tune per your scale
constexpr double pixels_per_meter = 1.0 / 1e6;
inline double metersPerPixel() { return 1.0 / pixels_per_meter; }

int main()
{
   
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    //-------------------------------INITIALIZE--------------------------
    sf::RenderWindow window(sf::VideoMode({ 2500, 1350 }), "OpenGL", sf::Style::Default, sf::State::Windowed, settings);
    window.setVerticalSyncEnabled(true);

    std::vector<Planet> planets;

    sf::Clock clock; // for delta time

    Planet planet1;
    planet1.position = { 900,700 };
    planet1.mass = 1.0e10;
    planet1.radius = 50;

    Planet planet2;
    planet2.position = { 1400,700 };
    planet2.mass = 1.0e10;
    planet2.radius = 50;

    Planet planet3;
    planet3.position = { 1900,700 };
    planet3.mass = 1.0e10;
    planet3.radius = 50;

    planets.push_back(planet1);
    planets.push_back(planet2);
    planets.push_back(planet3);

    //Main game loop
    while (window.isOpen())
    {

        sf::Vector2i localPosition = sf::Mouse::getPosition(window); // window is a sf::Window

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            {

                bool pressed = true;
                while (pressed)
                {
                    if (!(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)))
                    {
                        pressed = false;
                    }
                }
                sf::Vector2i pixel = sf::Mouse::getPosition(window);
                // left mouse button is pressed: Place circle
                planets.push_back(Planet{ static_cast<sf::Vector2f>(pixel), 30.f, 1.0e10 });
            }
        }

        // --- DELTA TIME ---
        float deltaTime = clock.restart().asMilliseconds(); // seconds since last frame

        std::vector<sf::Vector2f> planetAccelerations(planets.size(), { 0.f, 0.f });


        // Math to calculate acceleration between planet. This is calculated from each planet to all other planets.
        for (std::size_t x = 0; x < planets.size(); ++x)
        {
            for (std::size_t y = x + 1; y < planets.size(); ++y)
            {
                sf::Vector2f r = vectorFromPlanets(planets[x], planets[y]);
                double distance = std::sqrt(r.x * r.x + r.y * r.y); 
                if (distance == 0.f)
                {
                    continue;
                }

                long double force = calculateGravityForce(planets[x].mass, planets[y].mass, distance);

                sf::Vector2f unitVector = { r.x / static_cast<float>(distance), r.y/static_cast<float>(distance) };

                sf::Vector2f accelerationOnX = getVectorFromForce(planets[x].mass, force, unitVector);
                sf::Vector2f accelerationOnY = getVectorFromForce(planets[y].mass, force, -unitVector);

                planetAccelerations[x] += accelerationOnX;
                planetAccelerations[y] += accelerationOnY;

                std::cout << "Force: " << force << std::endl;

                /*doCircleCollision(planets[x], planets[y]);*/
                    
            }
        }

        for (std::size_t i = 0; i < planets.size(); ++i)
        {
            planets[i].velocity += planetAccelerations[i] * deltaTime;
            planets[i].position += planets[i].velocity * deltaTime;
        }

        window.clear(sf::Color::Black);

        
        for (Planet planet : planets)
        {
            sf::CircleShape shape(planet.radius);
            shape.setFillColor(sf::Color(100, 250, 50));
            shape.setPosition(sf::Vector2f(static_cast<float>(planet.position.x),static_cast<float>(planet.position.y)));
            shape.setOrigin({static_cast<float>(planet.radius), static_cast<float>(planet.radius)});
            window.draw(shape);
        }


        window.display();
    }
}







