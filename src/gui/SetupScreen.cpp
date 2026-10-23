#include "SetupScreen.h"
#include "engine/Piece.h"

namespace gui {

namespace {
const float LEFT = 60.f;
const char* const START_ROW = "rnbqkbnrpppppppp................................PPPPPPPPRNBQKBNR";
}

sf::FloatRect SetupScreen::backRect() const { return sf::FloatRect({40.f, 34.f}, {112.f, 42.f}); }
sf::FloatRect SetupScreen::themeRect(int i) const {
    return sf::FloatRect({LEFT + static_cast<float>(i) * 112.f, 176.f}, {100.f, 116.f});
}
sf::FloatRect SetupScreen::coordsRect() const { return toggleRect({LEFT, 346.f}); }
sf::FloatRect SetupScreen::hintsRect() const { return toggleRect({LEFT, 388.f}); }
sf::FloatRect SetupScreen::colorRect() const { return sf::FloatRect({LEFT, 468.f}, {330.f, 46.f}); }
sf::FloatRect SetupScreen::levelRect() const { return sf::FloatRect({LEFT, 556.f}, {450.f, 46.f}); }
sf::FloatRect SetupScreen::rotateRect() const { return toggleRect({LEFT, 468.f}); }
sf::FloatRect SetupScreen::startRect() const { return sf::FloatRect({LEFT, 664.f}, {548.f, 58.f}); }

void SetupScreen::handleEvent(const sf::Event& event) {
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        mouse_ = ctx_.toWorld(moved->position);
    } else if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (click->button == sf::Mouse::Button::Left) onClick(ctx_.toWorld(click->position));
    } else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) ctx_.goTo(ScreenId::Home);
        else if (key->code == sf::Keyboard::Key::Enter) ctx_.goTo(ScreenId::Game);
    }
}

void SetupScreen::onClick(sf::Vector2f p) {
    Settings& s = ctx_.settings;
    if (backRect().contains(p)) { ctx_.goTo(ScreenId::Home); return; }
    if (startRect().contains(p)) { ctx_.goTo(ScreenId::Game); return; }

    for (int i = 0; i < static_cast<int>(themes().size()); ++i) {
        if (themeRect(i).contains(p)) s.themeIndex = i;
    }
    if (coordsRect().contains(p)) s.showCoords = !s.showCoords;
    if (hintsRect().contains(p)) s.showHints = !s.showHints;

    if (vsComputer()) {
        int c = segmentedHit(colorRect(), 3, p);
        if (c >= 0) s.color = static_cast<ColorChoice>(c);
        int l = segmentedHit(levelRect(), 3, p);
        if (l >= 0) s.level = l;
    } else if (rotateRect().contains(p)) {
        s.rotateBoard = !s.rotateBoard;
    }
}

void SetupScreen::draw() {
    sf::RenderWindow& w = ctx_.window;
    const Fonts& f = ctx_.fonts;
    const Settings& s = ctx_.settings;
    drawBackground(w);

    // header
    drawButton(w, f, backRect(), "\xE2\x86\x90  Back", backRect().contains(mouse_));
    drawText(w, f.title, "Choose your layout", 36, {184.f, 46.f}, palette::text);
    drawText(w, f.text,
             vsComputer() ? "Playing the computer. Pick a look and how you want to play."
                          : "Two players on one screen. Pick a look for the board.",
             16, {184.f, 82.f}, palette::muted);

    // board theme cards
    drawText(w, f.bold, "BOARD THEME", 13, {LEFT, 150.f}, palette::muted);
    for (int i = 0; i < static_cast<int>(themes().size()); ++i) {
        sf::FloatRect r = themeRect(i);
        const bool selected = i == s.themeIndex;
        const bool hover = r.contains(mouse_);
        drawRoundedRect(w, r, 14.f, selected ? sf::Color(242, 184, 75, 30) : palette::card,
                        selected ? palette::accent : (hover ? sf::Color(255, 255, 255, 90) : palette::border),
                        selected ? 2.5f : 1.5f);
        drawMiniBoard(w, {r.position.x + 14.f, r.position.y + 12.f}, 18.f, 4, themes()[static_cast<std::size_t>(i)]);
        drawText(w, f.text, themes()[static_cast<std::size_t>(i)].name, 14,
                 {r.position.x + r.size.x / 2.f, r.position.y + 96.f}, palette::text, Align::Center);
    }

    // display options
    drawText(w, f.bold, "DISPLAY", 13, {LEFT, 322.f}, palette::muted);
    drawToggle(w, f, coordsRect().position, "Show coordinates", s.showCoords, coordsRect().contains(mouse_));
    drawToggle(w, f, hintsRect().position, "Highlight legal moves", s.showHints, hintsRect().contains(mouse_));

    if (vsComputer()) {
        drawText(w, f.bold, "PLAY AS", 13, {LEFT, 444.f}, palette::muted);
        drawSegmented(w, f, colorRect(), {"White", "Black", "Random"}, static_cast<int>(s.color),
                      segmentedHit(colorRect(), 3, mouse_));
        drawText(w, f.bold, "DIFFICULTY", 13, {LEFT, 532.f}, palette::muted);
        drawSegmented(w, f, levelRect(), {"Beginner", "Intermediate", "Advanced"}, s.level,
                      segmentedHit(levelRect(), 3, mouse_));
        drawText(w, f.text, levelHint(s.level), 15, {LEFT + 2.f, 626.f}, palette::muted);
    } else {
        drawText(w, f.bold, "TWO PLAYERS", 13, {LEFT, 444.f}, palette::muted);
        drawToggle(w, f, rotateRect().position, "Turn the board after every move", s.rotateBoard,
                   rotateRect().contains(mouse_));
    }

    drawButton(w, f, startRect(), "Start game", startRect().contains(mouse_), true);

    // live preview of the chosen theme
    const Theme& theme = themes()[static_cast<std::size_t>(s.themeIndex)];
    const float cell = 40.f;
    const sf::Vector2f origin(662.f, 176.f);
    drawRoundedRect(w, sf::FloatRect({origin.x - 14.f, origin.y - 14.f}, {cell * 8 + 28.f, cell * 8 + 28.f}), 18.f,
                    sf::Color(255, 255, 255, 14), palette::border, 1.f);
    drawMiniBoard(w, origin, cell, 8, theme);
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            char ch = START_ROW[r * 8 + c];
            if (ch == '.') continue;
            const bool white = ch >= 'A' && ch <= 'Z';
            PieceType type = PieceType::Pawn;
            switch (ch | 0x20) {
                case 'k': type = PieceType::King; break;
                case 'q': type = PieceType::Queen; break;
                case 'r': type = PieceType::Rook; break;
                case 'b': type = PieceType::Bishop; break;
                case 'n': type = PieceType::Knight; break;
                default: break;
            }
            drawPiece(w, f.symbols, type, white ? Color::White : Color::Black,
                      {origin.x + static_cast<float>(c) * cell + cell / 2.f,
                       origin.y + static_cast<float>(r) * cell + cell / 2.f}, 31);
        }
    }
    drawText(w, f.text, theme.name + " theme", 16, {origin.x + cell * 4.f, origin.y + cell * 8.f + 40.f},
             palette::muted, Align::Center);
}

}  // namespace gui
