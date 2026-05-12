#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <optional>
#include <iostream>
#include <vector>
#include <string>

enum class State { Menu, Growing };

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280, 700 }), "SFML Flower Project");
    window.setFramerateLimit(60);

    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("assets/background.png")) {
        std::cout << "Error loading assets/background.png\n";
    }
    sf::Sprite background(backgroundTexture);
    
    // Scale background to fit window (1280 / texture width, 700 / texture height)
    float scaleX = 1280.0f / backgroundTexture.getSize().x;
    float scaleY = 600.0f / backgroundTexture.getSize().y;
    background.setScale({scaleX, scaleY});

    std::vector<sf::Texture> stages(6);
    for (int i = 0; i < 6; i++) {
        std::string filename = "assets/" + std::to_string(i + 1) + ".png";
        if (!stages[i].loadFromFile(filename)) {
            std::cout << "Error loading " << filename << "\n";
        }
    }

    sf::Font font;
    if (!font.openFromFile("assets/arial.ttf")) { std::cout << "Error loading font\n"; }

    sf::Sprite flower(stages[0]);
    flower.setOrigin({ flower.getLocalBounds().size.x / 2.0f, flower.getLocalBounds().size.y });
    flower.setPosition({ 640.0f, 600.0f });

    sf::Text startButton(font, "CLICK TO GROW", 50);
    startButton.setOrigin({ startButton.getLocalBounds().size.x / 2.0f, startButton.getLocalBounds().size.y / 2.0f });
    startButton.setPosition({ 640.0f, 350.0f });
    startButton.setFillColor(sf::Color::White);

    sf::Clock growthClock;
    int currentStage = 0;
    float timer = 0.0f;
    float timePerStage = 3.0f; 
    State currentState = State::Menu;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (currentState == State::Menu && event->is<sf::Event::MouseButtonPressed>()) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (startButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                    currentState = State::Growing;
                    growthClock.restart(); 
                }
            }
        }

        if (currentState == State::Growing) {
            float dt = growthClock.restart().asSeconds();
            if (currentStage < 5) {
                timer += dt;
                if (timer >= timePerStage) {
                    currentStage++;
                    timer = 0.0f;
                    flower.setTexture(stages[currentStage]);
                    flower.setOrigin({ flower.getLocalBounds().size.x / 2.0f, flower.getLocalBounds().size.y });
                }
            }
        }

        window.clear(); 
        window.draw(background);

        if (currentState == State::Menu) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (startButton.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
                startButton.setFillColor(sf::Color::Yellow);
            } else {
                startButton.setFillColor(sf::Color::White);
            }
            window.draw(startButton);
        } 
        else if (currentState == State::Growing) {
            window.draw(flower);
        }

        window.display();
    }

    return 0;
}