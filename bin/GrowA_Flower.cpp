#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <optional>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cmath> 
#include <filesystem>
#include <cstdint>

enum class State { Menu, Selection, Growing };
enum class Weather { Sunny, Rainy };

void scaleBackgrounds(const sf::Vector2u& windowSize, const sf::Texture& texture, sf::Sprite& sun, sf::Sprite& night, sf::Sprite& rain) {
    float scaleX = (float)windowSize.x / texture.getSize().x;
    float scaleY = (float)windowSize.y / texture.getSize().y;
    for (auto* bg : {&sun, &night, &rain}) {
        bg->setScale({ scaleX, scaleY });
    }
}

void updateUIButtonPositions(const sf::Vector2u& windowSize, sf::Text& start, sf::Text& rose, sf::Text& sun, sf::Text& tulip) {
    float centerX = windowSize.x / 2.0f;
    start.setPosition({ centerX, windowSize.y / 2.0f });
    
    rose.setPosition({ centerX, windowSize.y * 0.35f }); // 35% down
    sun.setPosition({ centerX, windowSize.y * 0.50f });  // 50% down
    tulip.setPosition({ centerX, windowSize.y * 0.65f }); // 65% down
}

struct Raindrop {
    sf::RectangleShape shape;
    float speed;
    sf::Vector2u bounds;

    Raindrop(sf::Vector2u currentBounds) : bounds(currentBounds) { reset(); }

    void reset() {
        shape.setSize({ 2.0f, 12.0f });
        shape.setFillColor(sf::Color(100, 150, 255, 180));
        shape.setPosition({ (float)(std::rand() % bounds.x), (float)-(std::rand() % 100) });
        speed = 500.f + (std::rand() % 400);
    }

    void update(float dt) {
        shape.move({ 0.f, speed * dt });
        if (shape.getPosition().y > bounds.y) reset();
    }
    
    void updateBounds(sf::Vector2u newBounds) { bounds = newBounds; }
};

// Color ng mga buttons
sf::Color darkRed(150, 0, 0), darkYellow(180, 180, 0), darkCyan(0, 150, 150);
sf::Color brightRed = sf::Color::Red, brightYellow = sf::Color::Yellow, brightCyan = sf::Color::Cyan;

// NEW CUSTOM COLORS
sf::Color neonGreen(57, 255, 20);
sf::Color darkGreen(0, 100, 0);
sf::Color neonOrange(255, 95, 31);
sf::Color rustyOrange(183, 65, 14);

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode({ 1280, 700 }), "Flower Growth Simulator", sf::Style::Default);
    window.setFramerateLimit(60);

    // --- START UI BACKGROUND SECTION ---
    sf::Texture menuTex;
    if (!menuTex.loadFromFile("assets/cycle/menu_ui_bg.png")) return -1; 
    sf::Sprite menuBG(menuTex);

    sf::Texture sunTex, nightTex, rainTex;
    if (!sunTex.loadFromFile("assets/cycle/background.png")) return -1;
    if (!nightTex.loadFromFile("assets/cycle/background_night.png")) return -1; 
    if (!rainTex.loadFromFile("assets/cycle/background_rainy.png")) rainTex = sunTex; 

    sf::Sprite sunBG(sunTex), nightBG(nightTex), rainBG(rainTex);
    
    auto initialSize = window.getSize();
    scaleBackgrounds(initialSize, sunTex, sunBG, nightBG, rainBG);
    menuBG.setScale({ (float)initialSize.x / menuTex.getSize().x, (float)initialSize.y / menuTex.getSize().y });

    sf::Font font;
    if (!font.openFromFile("assets/font2p.ttf")) return -1;

   //Setup ng UI Texts
    sf::Text startButton(font, "START GAME", 30);
    sf::Text roseBtn(font, "ROSE", 25), sunBtn(font, "SUNFLOWER", 25), tulipBtn(font, "TULIP", 25);
    
    startButton.setOrigin({ startButton.getLocalBounds().size.x / 2.0f, startButton.getLocalBounds().size.y / 2.0f });
    for (auto* btn : {&roseBtn, &sunBtn, &tulipBtn}) {
        btn->setOrigin({ btn->getLocalBounds().size.x / 2.0f, btn->getLocalBounds().size.y / 2.0f });
    }
    
    updateUIButtonPositions(window.getSize(), startButton, roseBtn, sunBtn, tulipBtn);

    // MGA GAME OBJECTS ; sana di na magulo sa resize
    std::vector<sf::Texture> stages(6);
    sf::Sprite flower(sunTex); 
    flower.setPosition({ window.getSize().x / 2.0f, window.getSize().y * 0.92f }); // 92% down (base of screen)

    std::vector<Raindrop> rain;
    for(int i = 0; i < 500; ++i) rain.emplace_back(window.getSize());

    State currentState = State::Menu;
    Weather currentWeather = Weather::Sunny;
    std::string selectedFolder = "";
    
    sf::Clock mainClock;
    float weatherCheckTimer = 0.0f, growthTimer = 0.0f, totalTime = 0.0f, rainAlpha = 0.0f, nightAlpha = 0.0f, cycleLength = 120.0f;
    int currentStage = 0;

    while (window.isOpen()) {
        float dt = mainClock.restart().asSeconds();
        totalTime += dt;

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->is<sf::Event::MouseButtonPressed>()) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                if (currentState == State::Menu && startButton.getGlobalBounds().contains(mousePos)) {
                    currentState = State::Selection;
                }
                else if (currentState == State::Selection) {
                    bool choiceMade = false;
                    if (roseBtn.getGlobalBounds().contains(mousePos)) { selectedFolder = "rose"; choiceMade = true; }
                    if (sunBtn.getGlobalBounds().contains(mousePos)) { selectedFolder = "sunflower"; choiceMade = true; }
                    if (tulipBtn.getGlobalBounds().contains(mousePos)) { selectedFolder = "tulip"; choiceMade = true; }

                    if (choiceMade) {
                        for (int i = 0; i < 6; i++) {
                            std::string path = "assets/" + selectedFolder + "/" + std::to_string(i + 1) + ".png";
                            if (!stages[i].loadFromFile(path)) { std::cerr << "Missing: " << path << "\n"; }
                        }
                        currentStage = 0; growthTimer = 0.0f;
                        flower.setTexture(stages[0], true); 
                        flower.setOrigin({ flower.getLocalBounds().size.x / 2.0f, flower.getLocalBounds().size.y });
                        currentState = State::Growing;
                    }
                }
            }
            
            if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                sf::Vector2u newSize = resized->size;
                window.setView(sf::View(sf::FloatRect({ 0.f, 0.f }, { (float)newSize.x, (float)newSize.y })));
                
                scaleBackgrounds(newSize, sunTex, sunBG, nightBG, rainBG);
                menuBG.setScale({ (float)newSize.x / menuTex.getSize().x, (float)newSize.y / menuTex.getSize().y });
                
                updateUIButtonPositions(newSize, startButton, roseBtn, sunBtn, tulipBtn);
                flower.setPosition({ newSize.x / 2.0f, newSize.y * 0.92f });
                for(auto& drop : rain) drop.updateBounds(newSize);
            }
        }

        if (currentState == State::Growing) {
            nightAlpha = (std::sin(totalTime * (2.0f * 3.14159f / cycleLength)) * 0.5f + 0.5f) * 255.0f;
            nightBG.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(nightAlpha)));

            weatherCheckTimer += dt;
            if (weatherCheckTimer >= 60.0f) {
                weatherCheckTimer = 0;
                currentWeather = (std::rand() % 100 < 40) ? Weather::Rainy : Weather::Sunny;
            }

            if (currentWeather == Weather::Rainy) {
                if ((rainAlpha += 100.0f * dt) > 255) rainAlpha = 255;
                for (auto& drop : rain) drop.update(dt);
            } else if ((rainAlpha -= 100.0f * dt) < 0) rainAlpha = 0;
            
            rainBG.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(rainAlpha)));

            if (currentStage < 5) {
                growthTimer += dt * ((currentWeather == Weather::Rainy) ? 2.5f : 1.0f);
                if (growthTimer >= 45.0f) {
                    currentStage++; growthTimer = 0.0f;
                    flower.setTexture(stages[currentStage]);
                    flower.setOrigin({ flower.getLocalBounds().size.x / 2.0f, flower.getLocalBounds().size.y });
                }
            }

            int tintVal = 255 - static_cast<int>(nightAlpha * 0.4f);
            flower.setColor(sf::Color(tintVal, tintVal, tintVal + 20));

            if (selectedFolder == "rose") flower.setRotation(sf::degrees(std::sin(totalTime * 1.5f) * 2.0f));
            else if (selectedFolder == "tulip") flower.setRotation(sf::degrees(std::sin(totalTime * 3.0f) * 5.0f));
            else { float p = 1.0f + std::sin(totalTime * 2.0f) * 0.02f; flower.setScale({p, p}); }
        }

        window.clear();
        sf::Vector2f mPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (currentState == State::Menu) {
            window.draw(menuBG);
            // REVERSED: Neon Green base, Dark Green hover
            startButton.setFillColor(startButton.getGlobalBounds().contains(mPos) ? darkGreen : neonGreen);
            window.draw(startButton);
        } 
        else if (currentState == State::Selection) {
            window.draw(menuBG);
            
            // REVERSED ALL: Bright base, Dark hover
            roseBtn.setFillColor(roseBtn.getGlobalBounds().contains(mPos) ? darkRed : brightRed);
            sunBtn.setFillColor(sunBtn.getGlobalBounds().contains(mPos) ? darkYellow : brightYellow);
            tulipBtn.setFillColor(tulipBtn.getGlobalBounds().contains(mPos) ? rustyOrange : neonOrange);
            
            window.draw(roseBtn); window.draw(sunBtn); window.draw(tulipBtn);
        }
        else if (currentState == State::Growing) {
            window.draw(sunBG); window.draw(nightBG); window.draw(rainBG);   
            window.draw(flower);
            if (rainAlpha > 50) { for (auto& d : rain) window.draw(d.shape); }
        }
        window.display();
    }
    return 0;
}