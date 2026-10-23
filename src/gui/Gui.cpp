#include "Gui.h"
#include <algorithm>
#include <cmath>
#include <initializer_list>

namespace gui {

namespace {

const float PI = 3.14159265f;

char32_t glyphFor(PieceType t) {
    switch (t) {
        case PieceType::King:   return U'\u265A';
        case PieceType::Queen:  return U'\u265B';
        case PieceType::Rook:   return U'\u265C';
        case PieceType::Bishop: return U'\u265D';
        case PieceType::Knight: return U'\u265E';
        case PieceType::Pawn:   return U'\u265F';
    }
    return U'?';
}

sf::Color withAlpha(sf::Color c, int a) {
    c.a = static_cast<std::uint8_t>(std::clamp(a, 0, 255));
    return c;
}

}  // namespace

const std::vector<Theme>& themes() {
    static const std::vector<Theme> list = {
        {"Classic",  sf::Color(240, 217, 181), sf::Color(181, 136, 99)},
        {"Emerald",  sf::Color(238, 238, 210), sf::Color(118, 150, 86)},
        {"Ocean",    sf::Color(222, 227, 230), sf::Color(140, 162, 173)},
        {"Amethyst", sf::Color(230, 220, 242), sf::Color(143, 112, 184)},
        {"Midnight", sf::Color(139, 151, 168), sf::Color(63, 76, 94)},
    };
    return list;
}

const char* levelName(int level) {
    static const char* names[] = {"Beginner", "Intermediate", "Advanced"};
    return names[std::clamp(level, 0, 2)];
}

const char* levelHint(int level) {
    static const char* hints[] = {
        "Looks 1 move ahead. A good first opponent.",
        "Looks 2 moves ahead. Plays sensible chess.",
        "Looks 3 moves ahead. Takes a moment to think.",
    };
    return hints[std::clamp(level, 0, 2)];
}

bool Fonts::load() {
    auto tryOpen = [](sf::Font& f, std::initializer_list<const char*> paths) {
        for (const char* p : paths) {
            if (f.openFromFile(p)) return true;
        }
        return false;
    };
    // macOS paths first, then a Linux fallback
    bool ok = tryOpen(symbols, {"/System/Library/Fonts/Apple Symbols.ttf",
                                "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
                                "/Library/Fonts/Arial Unicode.ttf",
                                "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"});
    ok = tryOpen(text, {"/System/Library/Fonts/Supplemental/Arial.ttf",
                        "/Library/Fonts/Arial.ttf",
                        "/System/Library/Fonts/Helvetica.ttc",
                        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"}) && ok;
    if (!tryOpen(bold, {"/System/Library/Fonts/Supplemental/Arial Bold.ttf",
                        "/Library/Fonts/Arial Bold.ttf",
                        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"})) {
        bold = text;
    }
    if (!tryOpen(title, {"/System/Library/Fonts/Supplemental/Georgia Bold.ttf",
                         "/Library/Fonts/Georgia Bold.ttf",
                         "/System/Library/Fonts/Supplemental/Georgia.ttf"})) {
        title = bold;
    }
    return ok;
}

sf::String utf8(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

void drawText(sf::RenderTarget& t, const sf::Font& font, const std::string& s, unsigned size,
              sf::Vector2f pos, sf::Color color, Align align) {
    sf::Text text(font, utf8(s), size);
    text.setFillColor(color);
    sf::FloatRect b = text.getLocalBounds();
    float ox = b.position.x;
    if (align == Align::Center) ox += b.size.x / 2.f;
    else if (align == Align::Right) ox += b.size.x;
    text.setOrigin({ox, static_cast<float>(size) * 0.62f});
    text.setPosition({std::round(pos.x), std::round(pos.y)});
    t.draw(text);
}

void drawRoundedRect(sf::RenderTarget& t, sf::FloatRect r, float radius, sf::Color fill,
                     sf::Color outline, float thickness) {
    radius = std::min(radius, std::min(r.size.x, r.size.y) / 2.f);
    const int seg = 8;
    const sf::Vector2f centres[4] = {
        {r.position.x + r.size.x - radius, r.position.y + radius},
        {r.position.x + r.size.x - radius, r.position.y + r.size.y - radius},
        {r.position.x + radius,            r.position.y + r.size.y - radius},
        {r.position.x + radius,            r.position.y + radius},
    };
    const float start[4] = {-90.f, 0.f, 90.f, 180.f};

    sf::ConvexShape shape;
    shape.setPointCount(static_cast<std::size_t>(4 * (seg + 1)));
    std::size_t idx = 0;
    for (int k = 0; k < 4; ++k) {
        for (int i = 0; i <= seg; ++i) {
            float a = (start[k] + 90.f * static_cast<float>(i) / seg) * PI / 180.f;
            shape.setPoint(idx++, {centres[k].x + radius * std::cos(a), centres[k].y + radius * std::sin(a)});
        }
    }
    shape.setFillColor(fill);
    shape.setOutlineColor(outline);
    shape.setOutlineThickness(thickness);
    t.draw(shape);
}

void drawCard(sf::RenderTarget& t, sf::FloatRect r, bool hover, float radius) {
    drawRoundedRect(t, r, radius, hover ? palette::cardHover : palette::card,
                    hover ? palette::accent : palette::border, hover ? 2.f : 1.f);
}

void drawBackground(sf::RenderTarget& t) {
    const sf::Color top(33, 40, 62);
    const sf::Color bottom(13, 16, 24);
    sf::VertexArray bg(sf::PrimitiveType::TriangleStrip, 4);
    bg[0] = sf::Vertex{{0.f, 0.f}, top};
    bg[1] = sf::Vertex{{0.f, HEIGHT}, bottom};
    bg[2] = sf::Vertex{{WIDTH, 0.f}, top};
    bg[3] = sf::Vertex{{WIDTH, HEIGHT}, bottom};
    t.draw(bg);
    drawGlow(t, {WIDTH * 0.85f, HEIGHT * 0.95f}, 520.f, sf::Color(120, 70, 160, 46));
    drawGlow(t, {WIDTH * 0.1f, 0.f}, 560.f, sf::Color(70, 100, 190, 46));
}

void drawGlow(sf::RenderTarget& t, sf::Vector2f centre, float radius, sf::Color colour) {
    const int seg = 40;
    sf::VertexArray fan(sf::PrimitiveType::TriangleFan, static_cast<std::size_t>(seg + 2));
    fan[0] = sf::Vertex{centre, colour};
    sf::Color edge = withAlpha(colour, 0);
    for (int i = 0; i <= seg; ++i) {
        float a = 2.f * PI * static_cast<float>(i) / seg;
        fan[static_cast<std::size_t>(i + 1)] =
            sf::Vertex{{centre.x + radius * std::cos(a), centre.y + radius * std::sin(a)}, edge};
    }
    t.draw(fan);
}

void drawButton(sf::RenderTarget& t, const Fonts& f, sf::FloatRect r, const std::string& label,
                bool hover, bool primary, bool enabled) {
    sf::Color fill;
    sf::Color outline = sf::Color::Transparent;
    float thickness = 0.f;
    sf::Color textColor = palette::text;
    if (primary) {
        fill = hover ? palette::accentHover : palette::accent;
        textColor = palette::accentInk;
    } else {
        fill = hover && enabled ? palette::cardHover : palette::card;
        outline = hover && enabled ? palette::accent : palette::border;
        thickness = 1.5f;
    }
    if (!enabled) textColor = withAlpha(palette::muted, 130);
    drawRoundedRect(t, r, 12.f, fill, outline, thickness);
    drawText(t, f.bold, label, 17,
             {r.position.x + r.size.x / 2.f, r.position.y + r.size.y / 2.f}, textColor, Align::Center);
}

void drawSegmented(sf::RenderTarget& t, const Fonts& f, sf::FloatRect r,
                   const std::vector<std::string>& labels, int selected, int hover) {
    drawRoundedRect(t, r, 14.f, sf::Color(255, 255, 255, 18));
    const float w = r.size.x / static_cast<float>(labels.size());
    for (std::size_t i = 0; i < labels.size(); ++i) {
        sf::FloatRect seg({r.position.x + w * static_cast<float>(i) + 4.f, r.position.y + 4.f},
                          {w - 8.f, r.size.y - 8.f});
        bool on = static_cast<int>(i) == selected;
        if (on) drawRoundedRect(t, seg, 10.f, palette::accent);
        else if (static_cast<int>(i) == hover) drawRoundedRect(t, seg, 10.f, sf::Color(255, 255, 255, 22));
        drawText(t, on ? f.bold : f.text, labels[i], 16,
                 {seg.position.x + seg.size.x / 2.f, seg.position.y + seg.size.y / 2.f},
                 on ? palette::accentInk : palette::text, Align::Center);
    }
}

int segmentedHit(sf::FloatRect r, std::size_t count, sf::Vector2f p) {
    if (!r.contains(p)) return -1;
    int i = static_cast<int>((p.x - r.position.x) / (r.size.x / static_cast<float>(count)));
    return std::clamp(i, 0, static_cast<int>(count) - 1);
}

sf::FloatRect toggleRect(sf::Vector2f pos) {
    return sf::FloatRect(pos, {380.f, 36.f});
}

void drawToggle(sf::RenderTarget& t, const Fonts& f, sf::Vector2f pos, const std::string& label,
                bool on, bool hover) {
    sf::FloatRect track({pos.x, pos.y + 5.f}, {46.f, 26.f});
    drawRoundedRect(t, track, 13.f, on ? palette::accent : sf::Color(255, 255, 255, hover ? 46 : 36));
    sf::CircleShape knob(10.f);
    knob.setFillColor(sf::Color::White);
    knob.setPosition({pos.x + (on ? 25.f : 3.f) + 0.f, pos.y + 8.f});
    t.draw(knob);
    drawText(t, f.text, label, 17, {pos.x + 62.f, pos.y + 18.f}, palette::text);
}

void drawPiece(sf::RenderTarget& t, const sf::Font& symbols, PieceType type, Color color,
               sf::Vector2f centre, unsigned size) {
    sf::Text text(symbols, sf::String(glyphFor(type)), size);
    const bool white = color == Color::White;
    text.setFillColor(white ? sf::Color::White : sf::Color(22, 22, 22));
    text.setOutlineColor(white ? sf::Color(25, 25, 25) : sf::Color(235, 235, 235));
    text.setOutlineThickness(white ? 2.f : 1.f);
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    text.setPosition({std::round(centre.x), std::round(centre.y)});
    t.draw(text);
}

void drawMiniBoard(sf::RenderTarget& t, sf::Vector2f pos, float cell, int cells, const Theme& theme) {
    for (int r = 0; r < cells; ++r) {
        for (int c = 0; c < cells; ++c) {
            sf::RectangleShape sq({cell, cell});
            sq.setPosition({pos.x + static_cast<float>(c) * cell, pos.y + static_cast<float>(r) * cell});
            sq.setFillColor((r + c) % 2 == 0 ? theme.light : theme.dark);
            t.draw(sq);
        }
    }
}

}  // namespace gui
