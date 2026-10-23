// Shared pieces of the SFML interface: colours, themes, settings, fonts,
// the abstract Screen class and a few drawing helpers.
#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "engine/Types.h"

namespace gui {

constexpr float WIDTH = 1040.f;
constexpr float HEIGHT = 770.f;

namespace palette {
const sf::Color text(238, 240, 246);
const sf::Color muted(154, 163, 181);
const sf::Color accent(242, 184, 75);
const sf::Color accentHover(255, 205, 110);
const sf::Color accentInk(27, 20, 5);
const sf::Color card(255, 255, 255, 16);
const sf::Color cardHover(255, 255, 255, 30);
const sf::Color border(255, 255, 255, 36);
}  // namespace palette

// ---------------------------------------------------------------- settings
struct Theme {
    std::string name;
    sf::Color light;
    sf::Color dark;
};
const std::vector<Theme>& themes();

enum class Mode { VsComputer, TwoPlayers };
enum class ColorChoice { White, Black, Random };

struct Settings {
    Mode mode = Mode::VsComputer;
    int themeIndex = 0;
    bool showCoords = true;
    bool showHints = true;
    ColorChoice color = ColorChoice::White;
    int level = 1;               // 0 Beginner, 1 Intermediate, 2 Advanced
    bool rotateBoard = false;    // two players: turn the board after every move
};

constexpr int levelDepth(int level) { return level + 1; }   // how many moves ahead the AI looks
const char* levelName(int level);
const char* levelHint(int level);

// ------------------------------------------------------------------- fonts
struct Fonts {
    sf::Font symbols;   // contains the chess piece symbols
    sf::Font text;
    sf::Font bold;
    sf::Font title;
    bool load();
};

// ----------------------------------------------------------- screens
enum class ScreenId { Home, Setup, Game };

// What every screen needs, plus a way to ask the app to switch screens
struct Context {
    sf::RenderWindow& window;
    Fonts& fonts;
    Settings& settings;
    std::optional<ScreenId> pending;

    void goTo(ScreenId id) { pending = id; }
    sf::Vector2f toWorld(sf::Vector2i pixel) const { return window.mapPixelToCoords(pixel); }
};

// Abstract base class: Home, Setup and Game screens each override these
class Screen {
public:
    explicit Screen(Context& ctx) : ctx_(ctx) {}
    virtual ~Screen() = default;

    virtual void handleEvent(const sf::Event& event) = 0;
    virtual void update() {}
    virtual void draw() = 0;

protected:
    Context& ctx_;
};

std::unique_ptr<Screen> makeScreen(ScreenId id, Context& ctx);
int runApp();

// ---------------------------------------------------------------- drawing
enum class Align { Left, Center, Right };

sf::String utf8(const std::string& s);

// pos.y is the vertical centre of the text line
void drawText(sf::RenderTarget& t, const sf::Font& font, const std::string& s, unsigned size,
              sf::Vector2f pos, sf::Color color, Align align = Align::Left);

void drawRoundedRect(sf::RenderTarget& t, sf::FloatRect r, float radius, sf::Color fill,
                     sf::Color outline = sf::Color::Transparent, float thickness = 0.f);
void drawCard(sf::RenderTarget& t, sf::FloatRect r, bool hover = false, float radius = 18.f);
void drawBackground(sf::RenderTarget& t);
void drawGlow(sf::RenderTarget& t, sf::Vector2f centre, float radius, sf::Color colour);

void drawButton(sf::RenderTarget& t, const Fonts& f, sf::FloatRect r, const std::string& label,
                bool hover, bool primary = false, bool enabled = true);

void drawSegmented(sf::RenderTarget& t, const Fonts& f, sf::FloatRect r,
                   const std::vector<std::string>& labels, int selected, int hover);
int segmentedHit(sf::FloatRect r, std::size_t count, sf::Vector2f p);

sf::FloatRect toggleRect(sf::Vector2f pos);
void drawToggle(sf::RenderTarget& t, const Fonts& f, sf::Vector2f pos, const std::string& label,
                bool on, bool hover);

// Draws a chess piece symbol centred on `centre`
void drawPiece(sf::RenderTarget& t, const sf::Font& symbols, PieceType type, Color color,
               sf::Vector2f centre, unsigned size);

// A small board in the colours of a theme (used for the previews)
void drawMiniBoard(sf::RenderTarget& t, sf::Vector2f pos, float cell, int cells, const Theme& theme);

}  // namespace gui
