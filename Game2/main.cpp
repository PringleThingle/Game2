#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFML/OpenGL.hpp>
#include <math.h>
#include <vector>

//Stores information about planets, used for gravity calculations and movement.
struct Planet
{
    sf::Vector2f position;
    double radius;
    double mass; //MASS IN KG
    sf::Vector2f velocity;
    sf::Color color = sf::Color(rand() % 256, rand() % 256, rand() % 256);
};

//Function that divides a vector by a scalars
sf::Vector2f divideVectorByDouble(sf::Vector2f v1, double scalar) {

    return sf::Vector2f(v1.x / scalar, v1.y / scalar);
}


struct Menu {

    sf::RectangleShape backGround;
    sf::RectangleShape closeMenuButton;
    sf::Font menuFont;
    float windowLength;
    float windowHeight;

public:
    Menu() 
    {
        //---------------------BackGround-------------------------------
        backGround.setFillColor({ 77, 80, 84 });
        backGround.setOutlineThickness(5);
        backGround.setOutlineColor({ 16, 89, 201 });
        //---------------------------------------------------------------

        //---------------------BackGround-------------------------------
        closeMenuButton.setFillColor({ 181, 20, 44 });
        closeMenuButton.setOutlineThickness(1);
        closeMenuButton.setOutlineColor({ 150, 89, 84 });
        //---------------------------------------------------------------


    }

    void draw(sf::RenderWindow& window) {
        window.draw(backGround);
        window.draw(closeMenuButton);
    }

    void updateLayout(float windowLength, float windowHeight) {
        sf::Vector2f size(windowLength / 2.0f, windowHeight / 2.0f);
        backGround.setSize(size);
        backGround.setOrigin(size/2.0f);
        backGround.setPosition(size);

        closeMenuButton.setSize({ size.x / 30, size.x / 25 });
        closeMenuButton.setOrigin({ closeMenuButton.getSize().x, 0 });
        closeMenuButton.setPosition({ backGround.getPosition().x + size.x/2 - 5, backGround.getPosition().y - size.y/2 + 5});
    }

    //Array to hold all menu buttons
    //std::vector<sf::RectangleShape>menuButtons;
    //Vector to hold all menu text
    //std::vector<sf::Text>menuText;
    //sf::Text quitButton;
    //sf::RectangleShape quitButtonBackGround;

};

long double G = 6.6743e-11;

sf::Vector2u getDesktopResolution(int& horizontal, int& vertical) {
    RECT desktop;

    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);

    horizontal = desktop.right;
    vertical = desktop.bottom;

    return sf::Vector2u(horizontal, vertical);

}

//Calculates the gravitational force between 2 planets. Used to figure out planet accelerations / movement
long double calculateGravityForce(double mass1, double mass2, double distance)
{
        //Force = (G * mass1 * mass2) / distance^2 
        //DISTANCE BEING BETWEEN PLANET CENTERS IN METERS

    long double force = (G * mass1 * mass2) / (distance * distance);

    return force;
}

//Function to get the vector between 2 planets
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

//Dot product funct9on
inline float dot(const sf::Vector2f& a, const sf::Vector2f& b) { return a.x * b.x + a.y * b.y; }

inline float len(const sf::Vector2f& v) { return std::sqrt(dot(v, v)); }

//Function that multiplies 2 vectors together
template <typename T>
sf::Vector2<T> multiplyVectors(sf::Vector2<T> v1, sf::Vector2<T> v2) {

    return sf::Vector2<T>(v1.x * v2.x, v1.y * v2.y);
}

//Function that multiplies a vector by a scalar
sf::Vector2f multiplyVectorByDouble(sf::Vector2f v1, double scalar) {

    return sf::Vector2f(v1.x * scalar, v1.y * scalar);
}

//Function to calculate planet velocity after a collision with another planet
void doPlanetPlanetCollision(Planet& p1, Planet& p2, float restitution = 0.8f)
{
    float minimumDistance = p1.radius + p2.radius;
    float distBetweenPlanetCenters = std::sqrt((p2.position.x-p1.position.x)*(p2.position.x-p1.position.x) + (p2.position.y-p1.position.y)*(p2.position.y-p1.position.y));

    if (minimumDistance >= distBetweenPlanetCenters) {
        sf::Vector2f norm = (p2.position - p1.position) / distBetweenPlanetCenters;
        float pValue = (2 * (p1.velocity.x * norm.x + p1.velocity.y * norm.y - p2.velocity.x * norm.x - p2.velocity.y * norm.y))/(p1.mass+p2.mass);

        p1.velocity = (p1.velocity - multiplyVectorByDouble(norm, pValue * p1.mass)) * restitution;
        p2.velocity = (p2.velocity + multiplyVectorByDouble(norm, pValue * p2.mass)) * restitution;
    }
    else {
        return;
    }
}


//Function to prevent 2 planets from slowly sinking into each other once they are resting against each other
void preventSinking(Planet& p1, Planet& p2)
{
    const float minDist = p1.radius + p2.radius;
    sf::Vector2f d = p2.position - p1.position;
    float dist = len(d);
    if (dist >= minDist || dist == 0.f) return;

    sf::Vector2f n = d / dist; // contact normal
    float penetration = minDist - dist;

    // Move each planet out along the normal, weighted by mass
    float invA = (p1.mass > 0.f) ? 1.f / p1.mass : 0.f;
    float invB = (p2.mass > 0.f) ? 1.f / p2.mass : 0.f;

    const float slop = 0.01f; // ignore tiny overlap to avoid jitter
    const float percent = 0.8f; // 1.0 is push fully out (set 0.8 for softer)
    float corrMag = std::max(penetration - slop, 0.f) / (invA + invB) * percent;

    sf::Vector2f correction = corrMag * n;
    p1.position -= invA * correction;
    p2.position += invB * correction;

    // (optional) kill closing motion along the normal to keep them resting
    sf::Vector2f rv = p2.velocity - p1.velocity;
    float vn = rv.x * n.x + rv.y * n.y;
    if (vn < 0.f) {
        sf::Vector2f vnVec = vn * n;
        p1.velocity += invA * vnVec;
        p2.velocity -= invB * vnVec;
    }
}

constexpr double softening2 = 1e6;         // (meters^2) tune per your scale
constexpr double pixels_per_meter = 1.0 / 1e6;
inline double metersPerPixel() { return 1.0 / pixels_per_meter; }

int main()
{
   
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 4;

    //--------------------SCALING STUFF------------------------------------
    //-- Had to use this to get the windows scaling size (mine was set to 1.25 and was messing up the original
    //-- function to retrieve screen size). This scaling can be applied where actual screensize is needed
    auto activeWindow = GetActiveWindow();
    HMONITOR monitor = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);

    // Get the logical width and height of the monitor
    MONITORINFOEX monitorInfoEx;
    monitorInfoEx.cbSize = sizeof(monitorInfoEx);
    GetMonitorInfo(monitor, &monitorInfoEx);
    auto cxLogical = monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left;
    auto cyLogical = monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top;

    // Get the physical width and height of the monitor
    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);
    devMode.dmDriverExtra = 0;
    EnumDisplaySettings(monitorInfoEx.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
    auto cxPhysical = devMode.dmPelsWidth;
    auto cyPhysical = devMode.dmPelsHeight;

    // Calculate the scaling factor
    auto horizontalScale = ((double)cxPhysical / (double)cxLogical);
    auto verticalScale = ((double)cyPhysical / (double)cyLogical);
    //---------------------------------------------------------------------
    int horizontal = 0;
    int vertical = 0;

    sf::Vector2u screenResolution = getDesktopResolution(horizontal, vertical);



    //-------------------------------INITIALIZE--------------------------
    sf::RenderWindow window(sf::VideoMode({ screenResolution.x, screenResolution.y}), "OpenGL", sf::Style::Default, sf::State::Fullscreen, settings);
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);

    std::vector<Planet> planets;

    sf::Clock clock; // for delta time

    sf::Texture planetTexture;
    planetTexture.loadFromFile("Assets/rusts.jpg");
    planetTexture.setSmooth(true);

    Menu settingsMenu;

    //Main game loop
    while (window.isOpen())
    {

        //Constantly gets mouse position in the window
        sf::Vector2i localPosition = sf::Mouse::getPosition(window);

        // --- DELTA TIME ---
        float deltaTime;

        //While an event is happening
        while (const std::optional event = window.pollEvent())
        {

            //=================================================//
            //    .___  ___.  _______ .__   __.  __    __      //
            //    |   \/   | |   ____||  \ |  | |  |  |  |     //
            //    |  \  /  | |  |__   |   \|  | |  |  |  |     //
            //    |  |\/|  | |   __|  |  . `  | |  |  |  |     //
            //    |  |  |  | |  |____ |  |\   | |  `--'  |     //   
            //    |__|  |__| |_______||__| \__|  \______/      //
            //=================================================//  

            //This is some TERRIBLE TERRIBLE logic, pls find way to clean later. PLEASE
                                                                                                                                         
            bool menu = false;

            //If the event is the left mouse button
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            {

                bool pressed = true;
                while (pressed)
                {
                    if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)))
                    {
                        pressed = false;
                    }
                }

                if (menu == false) {
                    menu = true;
                }
            }

            while (menu == true) {
                deltaTime = 0;
                std::cout << "Menu open" << std::endl;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
                {
                    //Logic so menu isnt spammed
                    bool pressed = true;
                    while (pressed)
                    {
                        if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)))
                        {
                            pressed = false;
                        }
                    }

                    if (menu == true) {
                        menu = false;
                    }
                }

                float scaledLength = screenResolution.x * horizontalScale;
                float scaledHeight = screenResolution.y * horizontalScale;

                settingsMenu.windowLength = scaledLength;
                settingsMenu.windowHeight = scaledHeight;


                settingsMenu.updateLayout(scaledLength, scaledHeight);

                settingsMenu.draw(window);
                window.display();

                if (settingsMenu.closeMenuButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(sf::Mouse::getPosition()))) {
                    menu = false;
                }





            }

            //=======================================================================

            deltaTime = clock.restart().asMilliseconds(); // seconds since last frame

            //If the event is clicking on the X or Escape then the window closes
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
                bool moving = true;

            }

            //If the event is the left mouse button
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            {
                //Some logic so that the mouse input isn't spammed, planet only placed on RELEASE of mouse button
                bool pressed = true;
                while (pressed)
                {
                    if (!(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)))
                    {
                        pressed = false;
                    }
                }
                sf::Vector2i pixel = sf::Mouse::getPosition(window);
                // left mouse button is pressed: Place circle (Add the planet into an array with other planets which are then drawn later)
                planets.push_back(Planet{ static_cast<sf::Vector2f>(pixel), 50.f, 1.0e10, sf::Vector2f(0,0) });
               
            } 

            // window resize, however does not scale objects
            if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                sf::FloatRect visibleArea({ 0.f, 0.f }, sf::Vector2f(resized->size));
                window.setView(sf::View(visibleArea));
            }
        }

        //Planet accelerations stored and used later to update planet positions
        std::vector<sf::Vector2f> planetAccelerations(planets.size(), { 0.f, 0.f });

        deltaTime = clock.restart().asMilliseconds(); // seconds since last frame
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

                //std::cout << "Force: " << force << std::endl;  
            }
        }

        //----------------------------------------EDGE OF WINDOW COLLISION LOOP------------------------------------
        for (std::size_t i = 0; i < planets.size(); ++i)
        {
            if ((window.getSize().x < (planets[i].position.x + planets[i].radius)))
            {
                planets[i].position.x = window.getSize().x - planets[i].radius;
                planets[i].velocity.x = -planets[i].velocity.x;
            }
            if (((planets[i].position.x - planets[i].radius) < 0))
            {
                planets[i].position.x = planets[i].radius;
                planets[i].velocity.x = -planets[i].velocity.x;
            }

            if ((window.getSize().y < (planets[i].position.y + planets[i].radius)))
            {
                planets[i].position.y = window.getSize().y - planets[i].radius;
                planets[i].velocity.y = -planets[i].velocity.y;
            }

            if (((planets[i].position.y - planets[i].radius) < 0))
            {
                planets[i].position.y = planets[i].radius;
                planets[i].velocity.y = -planets[i].velocity.y;
            }
            planets[i].velocity += planetAccelerations[i] * deltaTime;
            planets[i].position += planets[i].velocity * deltaTime;
        }


        //Loop to calculate planet collisions with each other, and also prevent them phasing into each other
        for (std::size_t x = 0; x < planets.size(); ++x)
        {
            for (std::size_t y = x + 1; y < planets.size(); ++y)
            {
                doPlanetPlanetCollision(planets[x], planets[y]);
                preventSinking(planets[x], planets[y]);
            }
        }

        window.clear(sf::Color::Black);

        //Final loop to draw all the planets
        for (Planet planet : planets)
        {
            sf::CircleShape shape(planet.radius);
            shape.setPosition(sf::Vector2f(static_cast<float>(planet.position.x),static_cast<float>(planet.position.y)));
            shape.setOrigin({static_cast<float>(planet.radius), static_cast<float>(planet.radius)});
            shape.setFillColor(planet.color);
            //shape.setTexture(&planetTexture);
            window.draw(shape);
        }


        window.display();
    }
}







