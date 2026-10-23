#include <iostream>
#include "Gui.h"
#include "GameScreen.h"
#include "HomeScreen.h"
#include "SetupScreen.h"

namespace gui {

std::unique_ptr<Screen> makeScreen(ScreenId id, Context& ctx) {
    switch (id) {
        case ScreenId::Home:  return std::make_unique<HomeScreen>(ctx);
        case ScreenId::Setup: return std::make_unique<SetupScreen>(ctx);
        case ScreenId::Game:  return std::make_unique<GameScreen>(ctx);
    }
    return std::make_unique<HomeScreen>(ctx);
}

int runApp() {
    Fonts fonts;
    if (!fonts.load()) {
        std::cerr << "Could not load the fonts (looked for Apple Symbols and Arial).\n";
        return 1;
    }

    sf::ContextSettings contextSettings;
    contextSettings.antiAliasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned>(WIDTH), static_cast<unsigned>(HEIGHT)}),
                            "Chess Engine", sf::Style::Titlebar | sf::Style::Close, sf::State::Windowed,
                            contextSettings);
    window.setFramerateLimit(60);
    window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {WIDTH, HEIGHT})));

    Settings settings;
    Context ctx{window, fonts, settings, std::nullopt};
    std::unique_ptr<Screen> screen = makeScreen(ScreenId::Home, ctx);
    int fade = 0;                                   // frames left of the fade-in after a screen change

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            else screen->handleEvent(*event);
        }
        screen->update();

        if (ctx.pending) {
            screen = makeScreen(*ctx.pending, ctx);
            ctx.pending.reset();
            fade = 14;
        }

        window.clear();
        screen->draw();
        if (fade > 0) {
            sf::RectangleShape veil({WIDTH, HEIGHT});
            veil.setFillColor(sf::Color(13, 16, 24, static_cast<std::uint8_t>(255 * fade / 14)));
            window.draw(veil);
            --fade;
        }
        window.display();
    }
    return 0;
}

}  // namespace gui
