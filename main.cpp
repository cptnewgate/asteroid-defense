#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>

struct Rock {
    sf::RectangleShape shape;
    int hp = 1;
};

struct Medicine {
    sf::CircleShape shape;
    bool is_heal;
};

struct Bullet {
    sf::CircleShape shape;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Asteroid Defense");
    window.setFramerateLimit(60);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    bool paused = false;
    bool game_over = false;
    bool in_rage = false;

    sf::Font font;
    font.loadFromFile("arial.ttf");

    sf::Text timer_text, score_text, hp_text, high_score_text;
    timer_text.setFont(font);
    score_text.setFont(font);
    hp_text.setFont(font);
    high_score_text.setFont(font);
    timer_text.setCharacterSize(20);
    score_text.setCharacterSize(20);
    hp_text.setCharacterSize(20);
    high_score_text.setCharacterSize(20);
    timer_text.setFillColor(sf::Color::White);
    score_text.setFillColor(sf::Color::White);
    hp_text.setFillColor(sf::Color::White);
    high_score_text.setFillColor(sf::Color::White);
    timer_text.setPosition(10.f, 50.f);
    score_text.setPosition(10.f, 30.f);
    hp_text.setPosition(10.f, 70.f);
    high_score_text.setPosition(10.f, 90.f);

    int score = 0, high_score = 0, player_hp = 5;
    float speed = 3.f;
    const float speed_increment = 0.05f;

    sf::Clock game_clock, spawn_clock, shoot_clock;

    sf::ConvexShape player;
    player.setPointCount(3);
    player.setPoint(0, sf::Vector2f(0.f, 0.f));
    player.setPoint(1, sf::Vector2f(20.f, 0.f));
    player.setPoint(2, sf::Vector2f(10.f, -20.f));
    player.setFillColor(sf::Color::White);
    player.setOrigin(10.f, 10.f);
    player.setPosition(390.f, 580.f);

    sf::RectangleShape rage_beam(sf::Vector2f(10.f, -600.f));
    rage_beam.setFillColor(sf::Color::Red);
    rage_beam.setOrigin(5.f, 0.f);

    sf::CircleShape rage_core(10.f);
    rage_core.setFillColor(sf::Color::Red);
    rage_core.setOrigin(10.f, 10.f);

    std::vector<Rock> rocks;
    std::vector<Medicine> medicines;
    std::vector<Bullet> bullets;

    float spawn_interval = 1.5f;

    // Pause/Death screen elements
    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));

    sf::Text status_text, exit_text, restart_text;
    status_text.setFont(font);
    status_text.setCharacterSize(40);
    status_text.setFillColor(sf::Color::White);
    status_text.setPosition(400.f, 300.f);
    status_text.setOrigin(status_text.getLocalBounds().width / 2, 20.f);

    exit_text.setFont(font);
    exit_text.setCharacterSize(30);
    exit_text.setFillColor(sf::Color::White);
    exit_text.setString("Exit Game (Press Escape)");
    exit_text.setPosition(400.f, 360.f);
    exit_text.setOrigin(exit_text.getLocalBounds().width / 2, 15.f);

    restart_text.setFont(font);
    restart_text.setCharacterSize(30);
    restart_text.setFillColor(sf::Color::White);
    restart_text.setString("Restart Game (Press Enter)");
    restart_text.setPosition(400.f, 400.f);
    restart_text.setOrigin(restart_text.getLocalBounds().width / 2, 15.f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space && !game_over)
                paused = !paused;
            if (paused && event.key.code == sf::Keyboard::Escape)
                window.close();
            if (paused && event.key.code == sf::Keyboard::Enter) {
                score = 0;
                player_hp = 5;
                rocks.clear();
                medicines.clear();
                bullets.clear();
                game_clock.restart();
                spawn_clock.restart();
                paused = false;
                game_over = false;
                in_rage = false;
            }
        }

        if (!paused) {
            if (player_hp <= 0) {
                game_over = true;
                paused = true;
                continue;
            }

            float time = game_clock.getElapsedTime().asSeconds();
            std::stringstream time_ss;
            time_ss << "Time: " << static_cast<int>(time) << "s";
            timer_text.setString(time_ss.str());

            std::stringstream hp_ss;
            hp_ss << "HP: " << player_hp;
            hp_text.setString(hp_ss.str());

            speed = 3.f + time * speed_increment;

            if (spawn_clock.getElapsedTime().asSeconds() > spawn_interval) {
                Rock rock;
                rock.shape.setSize(sf::Vector2f(20.f, 20.f));
                rock.shape.setFillColor(sf::Color::Transparent);
                rock.shape.setOutlineColor(sf::Color::White);
                rock.shape.setOutlineThickness(2.f);
                float x = rand() % (800 - 20);
                rock.shape.setPosition(x, 0.f);
                rocks.push_back(rock);
                spawn_clock.restart();
                spawn_interval = 0.5f + static_cast<float>(rand() % 100) / 100.f;
            }

            // Movement
            float move_speed = in_rage ? 10.f : 3.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                player.move(-move_speed, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                player.move(move_speed, 0.f);

            // Shooting
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && shoot_clock.getElapsedTime().asMilliseconds() > 150) {
                if (in_rage) {
                    rage_core.setPosition(player.getPosition().x + 10.f, player.getPosition().y - 10.f);
                    rage_beam.setPosition(player.getPosition().x + 10.f, player.getPosition().y - 10.f);

                    for (auto it = rocks.begin(); it != rocks.end();) {
                        if (rage_beam.getGlobalBounds().intersects(it->shape.getGlobalBounds())) {
                            if (--it->hp <= 0) {
                                score++;
                                if (score > high_score) high_score = score;

                                // Drop medicine on rock destruction
                                if (rand() % 2 == 0) {
                                    Medicine med;
                                    med.shape.setRadius(10.f);
                                    med.shape.setOrigin(10.f, 10.f);
                                    med.shape.setPosition(it->shape.getPosition());
                                    if (rand() % 2 == 0) {
                                        med.is_heal = true;
                                        med.shape.setFillColor(sf::Color::Green);
                                    } else {
                                        med.is_heal = false;
                                        med.shape.setFillColor(sf::Color::Red);
                                    }
                                    medicines.push_back(med);
                                }

                                it = rocks.erase(it);
                                continue;
                            }
                        }
                        ++it;
                    }
                } else {
                    Bullet b;
                    b.shape.setRadius(5.f);
                    b.shape.setFillColor(sf::Color::White);
                    b.shape.setOrigin(5.f, 5.f);
                    b.shape.setPosition(player.getPosition().x + 10.f, player.getPosition().y - 10.f);
                    bullets.push_back(b);
                }
                shoot_clock.restart();
            }

            // Bullet movement
            for (auto it = bullets.begin(); it != bullets.end();) {
                it->shape.move(0.f, -10.f);
                bool hit = false;
                for (auto r = rocks.begin(); r != rocks.end(); ++r) {
                    if (it->shape.getGlobalBounds().intersects(r->shape.getGlobalBounds())) {
                        if (--r->hp <= 0) {
                            score++;
                            if (score > high_score) high_score = score;

                            // Drop medicine on rock destruction
                            if (rand() % 2 == 0) {
                                Medicine med;
                                med.shape.setRadius(10.f);
                                med.shape.setOrigin(10.f, 10.f);
                                med.shape.setPosition(r->shape.getPosition());
                                if (rand() % 2 == 0) {
                                    med.is_heal = true;
                                    med.shape.setFillColor(sf::Color::Green);
                                } else {
                                    med.is_heal = false;
                                    med.shape.setFillColor(sf::Color::Red);
                                }
                                medicines.push_back(med);
                            }

                            rocks.erase(r);
                        }
                        hit = true;
                        break;
                    }
                }
                if (hit || it->shape.getPosition().y < -10.f)
                    it = bullets.erase(it);
                else
                    ++it;
            }

            // Rock movement
            for (auto it = rocks.begin(); it != rocks.end();) {
                it->shape.move(0.f, speed);
                if (it->shape.getPosition().y > 600.f) {
                    if (!in_rage) player_hp--;
                    it = rocks.erase(it);
                } else {
                    ++it;
                }
            }

            // Medicine movement
            for (auto it = medicines.begin(); it != medicines.end();) {
                it->shape.move(0.f, speed);
                if (it->shape.getGlobalBounds().intersects(player.getGlobalBounds())) {
                    if (it->is_heal && player_hp < 5) player_hp++;
                    else if (!it->is_heal) in_rage = true;
                    it = medicines.erase(it);
                } else if (it->shape.getPosition().y > 600.f)
                    it = medicines.erase(it);
                else
                    ++it;
            }

            // Wrap player left/right
            auto pos = player.getPosition();
            if (pos.x > 800.f) pos.x = -20.f;
            if (pos.x < -20.f) pos.x = 800.f;
            player.setPosition(pos);
        }

        std::stringstream score_ss, high_ss;
        score_ss << "Score: " << score;
        high_ss << "High Score: " << high_score;
        score_text.setString(score_ss.str());
        high_score_text.setString(high_ss.str());

        window.clear();
        window.draw(timer_text);
        window.draw(score_text);
        window.draw(hp_text);
        window.draw(high_score_text);
        window.draw(player);
        for (const auto& r : rocks) window.draw(r.shape);
        for (const auto& m : medicines) window.draw(m.shape);
        for (const auto& b : bullets) window.draw(b.shape);
        if (in_rage) {
            window.draw(rage_beam);
            window.draw(rage_core);
        }

        if (paused) {
            status_text.setString(game_over ? "GAME OVER" : "PAUSED");
            window.draw(overlay);
            window.draw(status_text);
            window.draw(exit_text);
            window.draw(restart_text);
        }

        window.display();
    }

    return 0;
}