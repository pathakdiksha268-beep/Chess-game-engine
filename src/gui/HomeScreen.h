#pragma once
#include "Gui.h"

namespace gui {

// Screen 1: choose between playing the computer and two players
class HomeScreen : public Screen {
public:
    explicit HomeScreen(Context& ctx) : Screen(ctx) {}

    void handleEvent(const sf::Event& event) override;
    void draw() override;

private:
    sf::FloatRect cardRect(int index) const;
    sf::Vector2f mouse_{-1.f, -1.f};
};

}  // namespace gui
