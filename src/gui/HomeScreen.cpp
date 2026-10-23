#include "HomeScreen.h"

namespace gui {

sf::FloatRect HomeScreen::cardRect(int index) const {
    return sf::FloatRect({100.f + static_cast<float>(index) * 440.f, 340.f}, {400.f, 250.f});
}

void HomeScreen::handleEvent(const sf::Event& event) {
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        mouse_ = ctx_.toWorld(moved->position);
    } else if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (click->button != sf::Mouse::Button::Left) return;
        sf::Vector2f p = ctx_.toWorld(click->position);
        for (int i = 0; i < 2; ++i) {
            if (cardRect(i).contains(p)) {
                ctx_.settings.mode = (i == 0) ? Mode::VsComputer : Mode::TwoPlayers;
                ctx_.goTo(ScreenId::Setup);
            }
        }
    }
}

void HomeScreen::draw() {
    sf::RenderWindow& w = ctx_.window;
    const Fonts& f = ctx_.fonts;
    drawBackground(w);

    // logo, title and tagline
    drawGlow(w, {WIDTH / 2.f, 112.f}, 110.f, sf::Color(240, 160, 56, 60));
    sf::Text logo(f.symbols, sf::String(U'\u265E'), 92);
    logo.setFillColor(palette::accent);
    logo.setOutlineColor(sf::Color(110, 66, 10));
    logo.setOutlineThickness(2.f);
    const sf::FloatRect lb = logo.getLocalBounds();
    logo.setOrigin({lb.position.x + lb.size.x / 2.f, lb.position.y + lb.size.y / 2.f});
    logo.setPosition({WIDTH / 2.f, 108.f});
    w.draw(logo);
    drawText(w, f.title, "Chess Engine", 58, {WIDTH / 2.f, 214.f}, palette::text, Align::Center);
    drawText(w, f.text, "A chess engine written in C++ with a minimax AI.", 19,
             {WIDTH / 2.f, 268.f}, palette::muted, Align::Center);

    // the two mode cards
    const char* titles[2] = {"Play the Computer", "Two Players"};
    const char* line1[2] = {"Face the minimax engine.", "Pass and play on one screen."};
    const char* line2[2] = {"Choose your colour and difficulty.", "Every rule is enforced for both sides."};
    for (int i = 0; i < 2; ++i) {
        sf::FloatRect r = cardRect(i);
        const bool hover = r.contains(mouse_);
        if (hover) r.position.y -= 6.f;
        drawCard(w, r, hover, 22.f);

        const float x = r.position.x + 34.f;
        if (i == 0) {
            drawPiece(w, f.symbols, PieceType::King, Color::White, {x + 24.f, r.position.y + 64.f}, 58);
        } else {
            drawPiece(w, f.symbols, PieceType::Pawn, Color::White, {x + 18.f, r.position.y + 64.f}, 54);
            drawPiece(w, f.symbols, PieceType::Pawn, Color::Black, {x + 66.f, r.position.y + 64.f}, 54);
        }
        drawText(w, f.title, titles[i], 28, {x, r.position.y + 124.f}, palette::text);
        drawText(w, f.text, line1[i], 17, {x, r.position.y + 164.f}, palette::muted);
        drawText(w, f.text, line2[i], 17, {x, r.position.y + 190.f}, palette::muted);
        drawText(w, f.bold, "Choose  \xE2\x86\x92", 17, {x, r.position.y + 224.f}, palette::accent);
    }

    // feature chips
    const char* chips[4] = {"C++17 engine", "Minimax + alpha-beta pruning", "Piece-square evaluation",
                            "Castling, en passant, promotion"};
    const float widths[4] = {138.f, 252.f, 206.f, 270.f};
    float total = 3 * 12.f;
    for (float cw : widths) total += cw;
    float x = (WIDTH - total) / 2.f;
    for (int i = 0; i < 4; ++i) {
        sf::FloatRect r({x, 650.f}, {widths[i], 34.f});
        drawRoundedRect(w, r, 17.f, sf::Color(255, 255, 255, 12), palette::border, 1.f);
        drawText(w, f.text, chips[i], 14, {x + widths[i] / 2.f, 667.f}, palette::muted, Align::Center);
        x += widths[i] + 12.f;
    }
    drawText(w, f.text, "OOP mini project", 14, {WIDTH / 2.f, 727.f}, sf::Color(110, 118, 135), Align::Center);
}

}  // namespace gui
