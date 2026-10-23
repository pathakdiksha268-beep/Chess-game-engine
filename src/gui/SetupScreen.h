#pragma once
#include "Gui.h"

namespace gui {

// Screen 2: choose the layout (board theme) and how to play
class SetupScreen : public Screen {
public:
    explicit SetupScreen(Context& ctx) : Screen(ctx) {}

    void handleEvent(const sf::Event& event) override;
    void draw() override;

private:
    bool vsComputer() const { return ctx_.settings.mode == Mode::VsComputer; }

    sf::FloatRect backRect() const;
    sf::FloatRect themeRect(int index) const;
    sf::FloatRect coordsRect() const;
    sf::FloatRect hintsRect() const;
    sf::FloatRect colorRect() const;
    sf::FloatRect levelRect() const;
    sf::FloatRect rotateRect() const;
    sf::FloatRect startRect() const;

    void onClick(sf::Vector2f p);

    sf::Vector2f mouse_{-1.f, -1.f};
};

}  // namespace gui
