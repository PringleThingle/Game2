#include <SFML/Graphics.hpp>

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 0;

    //-------------------------------INITIALIZE--------------------------
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "SFML works!", sf::Style::Default, sf::State::Windowed, settings);
    sf::CircleShape shape(100.f);
    shape.setRadius(100.f);
    shape.setFillColor(sf::Color::Black);
    shape.setPosition({100, 100});
    shape.setOutlineThickness(5);
    shape.setOutlineColor(sf::Color::Blue);

    sf::RectangleShape rectangle({ 100.f, 100.f });
    rectangle.setFillColor(sf::Color::Magenta);
    rectangle.setPosition({ 200, 200 });
    rectangle.setOrigin(rectangle.getSize() / 2.f);
    rectangle.rotate(sf::degrees(45));

    //Main game loop
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        window.draw(shape);
        window.draw(rectangle);
        window.display();
    }
}







