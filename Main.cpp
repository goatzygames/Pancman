
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

struct Level {
    int id;
    std::string name;
    std::string filename;
    //sf::Texture previewTexture;
};

class Player {
    public:
    sf::Sprite sprite;
    // sf::Texture texture;
    // texture.loadFromFile("Assets/player-facing-right.png");
    sf::Vector2f velocity;
    bool onGround = false;

    Player() {
        // sprite.setTexture(texture);
        sprite.setPosition(100.0f, 300.0f);
    }

    void update(float indt) {
        velocity.y +=  981.0f * indt;
        sprite.move(velocity * indt);
    }
};

struct Tile {
    sf::Sprite sprite;
    int tile_id;
};

void loadLevel(const std::string& filename, std::vector<Tile>& tiles, sf::Texture& grass, sf::Texture& dirt, sf::Texture& goal, sf::Texture& empty, sf::Texture& spike) {
    std::ifstream file(filename);
    std::string line;

    int y = 0;
    while (std::getline(file, line)) {
            std::stringstream ss(line);
            int id;
            int x = 0;
            while (ss >> id) {
                if (id == 1 || id == 2 || id == 10 || id == 3 || id == 4) {
                    Tile tile;
                    tile.tile_id = id;
                    bool validTile = false;
                        // Grass Texture
                    if (id == 1) {
                        tile.sprite.setTexture(grass);
                        validTile = true;
                        // Dirt Texture
                    } else if (id == 2) {
                        tile.sprite.setTexture(dirt);
                        validTile = true;
                        // Goal Post
                    } else if (id == 10) {
                        tile.sprite.setTexture(goal);
                        validTile = true;
                        //tile.sprite.setScale(1.0f, 2.0f);
                    } else if (id == 3) {
                        tile.sprite.setTexture(spike);
                        validTile = true;
                    } else if (id == 4) {
                        tile.sprite.setTexture(empty);
                        validTile = true;
                    }
                    if (validTile) {
                    tile.sprite.setPosition(x * 64.0f, y * 64.0f);
                    tiles.push_back(tile);
                    }
                }
                x++;
            }
            y++;
    }
}

int main() {

    // Window settings
    sf::RenderWindow window(sf::VideoMode(1280, 704), "Pancman", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(120);
    sf::View view(sf::FloatRect(0, 0, 1280, 704));

    // Level Stuff
    std::vector<Level> LevelList;
    LevelList.push_back({0, "FirstLevel", "Levels/level1.txt"});
    LevelList.push_back({1, "SecondLevel", "Levels/level2.txt"});
    LevelList.push_back({2, "ThirdLevel", "Levels/level3.txt"});
    LevelList.push_back({3, "FourthtLevel", "Levels/level4.txt"});
    LevelList.push_back({4, "FifthLevel", "Levels/level5.txt"});
    LevelList.push_back({5, "SixthLevel", "Levels/level6.txt"});

    float startY = 100.0f;
    float buttonHeight = 80.0f;
    float spacing = 15.0f;
    float scrollOffset = 0.0f;
    int currentPage = 0;
    const int levelsPerPage = 5;
    int totalLevels = LevelList.size();
    int totalPages = std::ceil(totalLevels / static_cast<float>(levelsPerPage));

    // Clock
    sf::Clock clock;

    // Variables
    float Cooldown;
    enum class GameState {Menu, LevelChoose, Playing};
    GameState currentState = GameState::LevelChoose;
    bool canJump = false;

    sf::Vector2f playerPos;
    sf::Vector2f viewCenter;

    std::vector<Tile> tiles;

    sf::Texture grass;
    sf::Texture dirt;
    sf::Texture sky;
    sf::Texture testgoal;
    sf::Texture spiketex;
    sf::Texture emptytex;

    grass.loadFromFile("Assets/grass.png");
    dirt.loadFromFile("Assets/dirt.png");
    sky.loadFromFile("Assets/sky-platformer.png");
    testgoal.loadFromFile("Assets/goalpost.png");
    spiketex.loadFromFile("Assets/spike64.png");
    emptytex.loadFromFile("Assets/empty64.png");

    sf::Sprite skySprite;
    skySprite.setTexture(sky);
    sf::Vector2u skytexSize = sky.getSize();
    sf::Vector2u winSize = window.getSize();
    float skyScaleX = float(winSize.x) / skytexSize.x;
    float skyScaleY = float(winSize.y) / skytexSize.y;
    skySprite.setScale(skyScaleX, skyScaleY);
    sf::Vector2u skySize = sky.getSize();
    skySprite.setOrigin(skySize.x / 2.0f, skySize.y / 2.0f);

    Player player;

    player.sprite.setPosition(100, 100);
    float viewCenterY = player.sprite.getPosition().y - 2.0f;
    player.velocity = {0, 0};
    canJump = true;
    sf::Texture playerTextureRight;
    playerTextureRight.loadFromFile("Assets/pancake-man-right.png");
    player.sprite.setTexture(playerTextureRight);
    sf::Texture playerTextureLeft;
    playerTextureLeft.loadFromFile("Assets/pancake-man-left.png");
    // Animation sprites
    sf::Texture playerSpring1;
    playerSpring1.loadFromFile("Assets/pancake-spring1.png");
    sf::Texture playerSpring2;
    playerSpring2.loadFromFile("Assets/pancake-spring2.png");
    sf::Texture playerSpring3;
    playerSpring3.loadFromFile("Assets/pancake-spring3.png");

    // Topbar
    sf::Texture topbarTex;
    topbarTex.loadFromFile("Assets/topbar.png");
    sf::RectangleShape topbar;
    topbar.setTexture(&topbarTex);
    topbar.setSize(sf::Vector2f(1280, 80));

    // Menu UI
    // Title
    sf::Text title;
    title.setString("Pancman");
    sf::Font font;
    if (!font.loadFromFile("Assets/Font/Pixellari.ttf")) {
        std::printf("Whaat");
    };
        sf::Text prevText(" < ", font, 30);
        sf::Text nextText(" > ", font, 30);

    loadLevel("Levels/level1.txt", tiles, grass, dirt, testgoal, emptytex, spiketex);

    // While the window is open
    while (window.isOpen()) {
        sf::Event event;

        sf::Vector2u windowSize = window.getSize();
        unsigned int windowWidth = windowSize.x;
        unsigned int windowHeight = windowSize.y;

        float dt = clock.restart().asSeconds();
        Cooldown -= dt;
        if (Cooldown < 0.0f) {
            Cooldown = 0.0f;
        }

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }


            // Mouse events
            if (currentState == GameState::LevelChoose) {
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                if (prevText.getGlobalBounds().contains(mousePos)) {
                    if (currentPage > 0) {
                        currentPage--;
                    }
                }
                     else if (nextText.getGlobalBounds().contains(mousePos)) {
                        if (currentPage < totalPages - 1) {
                            currentPage++;
                        }
                    }
                    int startIndex12 = currentPage * levelsPerPage;
                    int endIndex12 = std::min(startIndex12 + levelsPerPage, totalLevels);
                    for (int i = startIndex12; i < endIndex12; ++i) {
                        int displayIndex = i - startIndex12;
                        float y = startY + displayIndex * (buttonHeight + spacing);
                        sf::FloatRect buttonRect(100.0f, y, windowWidth - 200.0f, buttonHeight);
                        if (buttonRect.contains(mousePos)) {
                            tiles.clear();
                            currentState = GameState::Playing;
                            loadLevel(LevelList[i].filename, tiles, grass, dirt, testgoal, emptytex, spiketex);
                            break;
                        }
                    }
            }
        }
    }

        playerPos = player.sprite.getPosition();
        viewCenter = view.getCenter();
        viewCenter.x = playerPos.x;
        viewCenter.y = playerPos.y;
        view.setCenter(viewCenter);
        
        skySprite.setPosition(viewCenter);
        topbar.setPosition(viewCenter.x - (1280 / 2.0f), viewCenter.y - (704 / 2.0f));

        window.setView(view);

        player.update(dt);

        // Essential things end

        // Character movement

        // If in Playing Mode
    if (currentState == GameState::Playing) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || (sf::Keyboard::isKeyPressed(sf::Keyboard::D))) {
            player.velocity.x = 200.0f;
            player.sprite.setTexture(playerTextureRight);

        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || (sf::Keyboard::isKeyPressed(sf::Keyboard::A))) {
            player.velocity.x = -200.0f;
            player.sprite.setTexture(playerTextureLeft);

        } else {
            player.velocity.x = 0.0f;
        }

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))) {
            if (player.onGround) {
            player.velocity.y = -550.0f;
            player.onGround = false;
            }
        }

    }

        // Tile collisions
        for (auto& tile : tiles) {
            sf::FloatRect playerBounds = player.sprite.getGlobalBounds();
            sf::FloatRect tileBounds = tile.sprite.getGlobalBounds();

            // Special collisions
            if (playerBounds.intersects(tileBounds) && tile.tile_id == 10) {

                window.close();
            }

            // Normal collisions
            if (playerBounds.intersects(tileBounds)) {
                sf::FloatRect overlap;
                overlap.left = std::max(playerBounds.left, tileBounds.left);
                overlap.top = std::max(playerBounds.top, tileBounds.top);
                overlap.width = std::min(playerBounds.left + playerBounds.width, tileBounds.left + tileBounds.width) - overlap.left;
                overlap.height = std::min(playerBounds.top + playerBounds.height, tileBounds.top + tileBounds.height) - overlap.top;

                if (player.velocity.y > 0 && (playerBounds.top + playerBounds.height - player.velocity.y * dt) <= tileBounds.top) {
                    player.sprite.setPosition(player.sprite.getPosition().x, tileBounds.top - playerBounds.height);
                    player.velocity.y = 0;
                    player.onGround = true;
                } else if (player.velocity.y < 0 && (playerBounds.top >= tileBounds.top + tileBounds.height - 5)) {
                    player.sprite.setPosition(player.sprite.getPosition().x, tileBounds.top + tileBounds.height);
                    player.velocity.y = 0;
                } else if (overlap.width < overlap.height) {
                    if (playerBounds.left < tileBounds.left) {
                        player.sprite.move(-overlap.width, 0);
                    } else {
                        player.sprite.move(overlap.width, 0);
                    }
                    player.velocity.x = 0;
                }
            }
        }


        // Draw correct things
        if (currentState == GameState::Menu) {
            window.clear();
            window.draw(skySprite);


        } else if (currentState == GameState::Playing) {
            window.clear();
            window.draw(skySprite);

            for (auto& tile : tiles) {
                window.draw(tile.sprite);
            }
            window.draw(player.sprite);
        } else if (currentState == GameState::LevelChoose) {
            window.clear();
            window.draw(skySprite);
        
             // Level list
            int startIndex = currentPage * levelsPerPage;
            int endIndex = std::min(startIndex + levelsPerPage, totalLevels);

            for (int i = startIndex; i < endIndex; ++i) {
            int displayIndex = i - startIndex;
            float y = startY + displayIndex * (buttonHeight + spacing);

            sf::RectangleShape levelButton(sf::Vector2f(windowWidth - 200.0f, buttonHeight));
            levelButton.setPosition(100.0f, y);
            levelButton.setFillColor(sf::Color(80, 80, 80));

            sf::Text levelNameText;
            levelNameText.setFont(font);
            levelNameText.setString(LevelList[i].name);
            levelNameText.setPosition(120.0f, y + 20.0f);

            window.draw(levelButton);
            window.draw(levelNameText);

            prevText.setPosition(100, windowHeight - 100);
            nextText.setPosition(windowWidth - 200, windowHeight - 100);

            window.draw(prevText);
            window.draw(nextText);
            }
        }

        window.draw(topbar);
        window.display();


    }
}