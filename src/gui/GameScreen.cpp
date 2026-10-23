#include "GameScreen.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace gui {

namespace {

const float ANIM_SECONDS = 0.16f;
const int VISIBLE_ROWS = 10;

Color other(Color c) { return c == Color::White ? Color::Black : Color::White; }

char fileLetter(int col) { return static_cast<char>('a' + col); }
char rankDigit(int row) { return static_cast<char>('0' + (8 - row)); }

std::string squareName(Position p) {
    return std::string(1, fileLetter(p.col)) + rankDigit(p.row);
}

char pieceLetter(PieceType t) {
    switch (t) {
        case PieceType::King:   return 'K';
        case PieceType::Queen:  return 'Q';
        case PieceType::Rook:   return 'R';
        case PieceType::Bishop: return 'B';
        case PieceType::Knight: return 'N';
        default:                return 'P';
    }
}

// the order captured pieces are shown in, and where each piece type sits in that order
const PieceType SHOW_ORDER[5] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop,
                                 PieceType::Knight, PieceType::Pawn};
const int SLOT[6] = {4, 3, 2, 1, 0, -1};     // indexed by PieceType: Pawn, Knight, Bishop, Rook, Queen, King

}  // namespace

// ------------------------------------------------------------------ set-up

GameScreen::GameScreen(Context& ctx)
    : Screen(ctx), ai_(levelDepth(ctx.settings.level)) {
    vsAI_ = ctx.settings.mode == Mode::VsComputer;
    startNewGame();
}

void GameScreen::startNewGame() {
    board_.setupStartingPosition();
    turn_ = Color::White;
    history_.clear();
    last_.reset();
    selected_.reset();
    targets_.clear();
    anim_.active = false;
    overlay_ = Overlay::None;
    overlayDelay_ = 0;
    aiPending_ = false;
    scrollRows_ = 0;

    if (vsAI_) {
        ColorChoice choice = ctx_.settings.color;
        if (choice == ColorChoice::Random) {
            static std::mt19937 rng{std::random_device{}()};
            choice = (rng() & 1u) ? ColorChoice::Black : ColorChoice::White;
        }
        human_ = (choice == ColorChoice::White) ? Color::White : Color::Black;
        flipped_ = human_ == Color::Black;          // keep your pieces at the bottom
    } else {
        flipped_ = false;
    }
    refresh();
    maybeStartAi();                                  // the computer opens if you chose Black
}

void GameScreen::refresh() {
    legal_ = board_.getLegalMoves(turn_);
    const bool check = board_.isInCheck(turn_);
    if (legal_.empty()) status_ = check ? 2 : 3;
    else status_ = check ? 1 : 0;
}

// ------------------------------------------------------------------ layout

sf::FloatRect GameScreen::barRect(bool top) const {
    return sf::FloatRect({BOARD_X, top ? 8.f : 720.f}, {SQ * 8.f, 42.f});
}

sf::FloatRect GameScreen::buttonRect(int index) const {
    return sf::FloatRect({720.f + static_cast<float>(index % 2) * 146.f, 530.f + static_cast<float>(index / 2) * 58.f},
                         {134.f, 46.f});
}

sf::FloatRect GameScreen::overlayCard() const {
    sf::Vector2f size(380.f, 180.f);
    if (overlay_ == Overlay::GameOver) size = {420.f, 300.f};
    else if (overlay_ == Overlay::Promotion) size = {380.f, 200.f};
    return sf::FloatRect({(WIDTH - size.x) / 2.f, (HEIGHT - size.y) / 2.f}, size);
}

sf::FloatRect GameScreen::overlayButton(int index) const {
    const sf::FloatRect c = overlayCard();
    const float x = c.position.x;
    const float y = c.position.y;
    switch (overlay_) {
        case Overlay::GameOver:
            if (index == 0) return sf::FloatRect({x + 40.f, y + 176.f}, {340.f, 46.f});
            if (index == 1) return sf::FloatRect({x + 40.f, y + 232.f}, {164.f, 42.f});
            return sf::FloatRect({x + 216.f, y + 232.f}, {164.f, 42.f});
        case Overlay::Promotion:
            return sf::FloatRect({x + 20.f + static_cast<float>(index) * 88.f, y + 92.f}, {76.f, 76.f});
        default:
            if (index == 0) return sf::FloatRect({x + 30.f, y + 108.f}, {150.f, 46.f});
            return sf::FloatRect({x + 200.f, y + 108.f}, {150.f, 46.f});
    }
}

sf::Vector2f GameScreen::squarePos(int row, int col) const {
    const int dr = flipped_ ? 7 - row : row;
    const int dc = flipped_ ? 7 - col : col;
    return {BOARD_X + static_cast<float>(dc) * SQ, BOARD_Y + static_cast<float>(dr) * SQ};
}

std::optional<Position> GameScreen::squareAt(sf::Vector2f p) const {
    if (p.x < BOARD_X || p.y < BOARD_Y || p.x >= BOARD_X + SQ * 8.f || p.y >= BOARD_Y + SQ * 8.f) return std::nullopt;
    const int dc = static_cast<int>((p.x - BOARD_X) / SQ);
    const int dr = static_cast<int>((p.y - BOARD_Y) / SQ);
    return Position{flipped_ ? 7 - dr : dr, flipped_ ? 7 - dc : dc};
}

// -------------------------------------------------------------- game logic

bool GameScreen::animating() const {
    return anim_.active && animClock_.getElapsedTime().asSeconds() < ANIM_SECONDS;
}

bool GameScreen::humanCanMove() const {
    if (overlay_ != Overlay::None || status_ >= 2 || aiPending_) return false;
    return !vsAI_ || turn_ == human_;
}

// The move text before the check / mate suffix is known: e4, Nf3, exd6, O-O ...
std::string GameScreen::sanBase(const Move& m) const {
    if (m.type == MoveType::Castle) return m.to.col > m.from.col ? "O-O" : "O-O-O";

    const Piece* piece = board_.getPiece(m.from);
    const PieceType kind = piece->type();
    const bool capture = !board_.isEmpty(m.to) || m.type == MoveType::EnPassant;

    std::string s;
    if (kind == PieceType::Pawn) {
        if (capture) { s += fileLetter(m.from.col); s += 'x'; }
        s += squareName(m.to);
        if (m.type == MoveType::Promotion) { s += '='; s += pieceLetter(m.promotion); }
        return s;
    }

    s += pieceLetter(kind);
    // if two identical pieces can reach the square, say which one moved
    bool rival = false, sameFile = false, sameRank = false;
    for (const Move& o : legal_) {
        if (!(o.to == m.to) || o.from == m.from) continue;
        const Piece* p = board_.getPiece(o.from);
        if (p == nullptr || p->type() != kind || p->color() != piece->color()) continue;
        rival = true;
        if (o.from.col == m.from.col) sameFile = true;
        if (o.from.row == m.from.row) sameRank = true;
    }
    if (rival) {
        if (!sameFile) s += fileLetter(m.from.col);
        else if (!sameRank) s += rankDigit(m.from.row);
        else { s += fileLetter(m.from.col); s += rankDigit(m.from.row); }
    }
    if (capture) s += 'x';
    s += squareName(m.to);
    return s;
}

void GameScreen::playMove(const Move& m) {
    std::string san = sanBase(m);                  // needs the position before the move
    board_.makeMove(m);
    turn_ = other(turn_);
    anim_ = {m.from, m.to, true};
    animClock_.restart();
    refresh();
    if (status_ == 2) san += '#';
    else if (status_ == 1) san += '+';

    history_.push_back({m, san});
    last_ = m;
    selected_.reset();
    targets_.clear();
    scrollRows_ = 0;
    if (!vsAI_ && ctx_.settings.rotateBoard) flipped_ = turn_ == Color::Black;

    if (status_ >= 2) overlayDelay_ = 36;          // show the final position for a moment first
    else maybeStartAi();
}

void GameScreen::maybeStartAi() {
    if (vsAI_ && status_ < 2 && turn_ != human_) {
        aiPending_ = true;
        aiDelay_ = 18;                             // lets your own move be drawn first
    }
}

void GameScreen::update() {
    if (anim_.active && !animating()) anim_.active = false;

    if (aiPending_ && !animating() && --aiDelay_ <= 0) {
        aiPending_ = false;
        Move m = ai_.findBestMove(board_, turn_);  // blocks for a moment at higher difficulty
        playMove(m);
    }
    if (overlayDelay_ > 0 && !animating() && --overlayDelay_ == 0) overlay_ = Overlay::GameOver;
}

// How many half-moves undo takes back (0 = not possible right now)
int GameScreen::undoSteps() const {
    if (aiPending_ || history_.empty()) return 0;
    const int length = static_cast<int>(history_.size());
    if (vsAI_) {
        const int steps = (turn_ == human_) ? 2 : 1;               // back to your previous turn
        const int minLength = (human_ == Color::Black) ? 1 : 0;    // the computer's opening move stays
        return length - steps >= minLength ? steps : 0;
    }
    return 1;
}

void GameScreen::undo() {
    const int steps = undoSteps();
    if (steps == 0) return;
    for (int i = 0; i < steps; ++i) {
        board_.undoMove();
        history_.pop_back();
        turn_ = other(turn_);
    }
    refresh();
    last_ = history_.empty() ? std::nullopt : std::optional<Move>(history_.back().move);
    selected_.reset();
    targets_.clear();
    anim_.active = false;
    overlay_ = Overlay::None;
    overlayDelay_ = 0;
    if (!vsAI_ && ctx_.settings.rotateBoard) flipped_ = turn_ == Color::Black;
}

GameScreen::Material GameScreen::material() const {
    int cnt[2][6] = {};
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = board_.getPiece({r, c});
            if (p != nullptr) cnt[static_cast<int>(p->color())][static_cast<int>(p->type())]++;
        }
    }
    const int start[6] = {8, 2, 2, 2, 1, 1};
    const int worth[6] = {1, 3, 3, 5, 9, 0};
    Material m{};
    int total[2] = {0, 0};
    for (int side = 0; side < 2; ++side) {
        int extra = 0;                                              // promoted pieces
        for (int t = 1; t <= 4; ++t) {
            const int d = start[t] - cnt[side][t];
            if (d > 0) m.captured[side][SLOT[t]] = d;
            else extra += -d;
        }
        m.captured[side][SLOT[0]] = std::max(0, 8 - cnt[side][0] - extra);   // a promoted pawn is not captured
        for (int t = 0; t < 6; ++t) total[side] += cnt[side][t] * worth[t];
    }
    m.lead = total[0] - total[1];
    return m;
}

std::string GameScreen::playerName(Color c) const {
    if (vsAI_) {
        return c == human_ ? "You" : std::string("Computer \xC2\xB7 ") + levelName(ctx_.settings.level);
    }
    return c == Color::White ? "White" : "Black";
}

// ------------------------------------------------------------------- input

void GameScreen::handleEvent(const sf::Event& event) {
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        mouse_ = ctx_.toWorld(moved->position);
    } else if (const auto* click = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (click->button == sf::Mouse::Button::Left) {
            mouse_ = ctx_.toWorld(click->position);
            onClick(mouse_);
        }
    } else if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        if (movesRect().contains(ctx_.toWorld(wheel->position))) {
            const int rows = static_cast<int>((history_.size() + 1) / 2);
            const int maxScroll = std::max(0, rows - VISIBLE_ROWS);
            scrollRows_ = std::clamp(scrollRows_ + (wheel->delta > 0 ? 1 : -1), 0, maxScroll);
        }
    } else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            if (overlay_ == Overlay::GameOver || overlay_ == Overlay::Promotion || overlay_ == Overlay::Confirm) {
                overlay_ = Overlay::None;
            } else {
                selected_.reset();
                targets_.clear();
            }
        } else if (key->code == sf::Keyboard::Key::U && overlay_ == Overlay::None) {
            undo();
        }
    }
}

void GameScreen::onClick(sf::Vector2f p) {
    if (overlay_ != Overlay::None) { onOverlayClick(p); return; }

    for (int i = 0; i < 4; ++i) {
        if (!buttonRect(i).contains(p)) continue;
        const bool needsConfirm = !history_.empty() && status_ < 2;
        if (i == 0) undo();
        else if (i == 1) flipped_ = !flipped_;
        else if (i == 2) {
            if (needsConfirm) { confirm_ = ConfirmAction::NewGame; overlay_ = Overlay::Confirm; }
            else startNewGame();
        } else {
            if (needsConfirm) { confirm_ = ConfirmAction::Menu; overlay_ = Overlay::Confirm; }
            else ctx_.goTo(ScreenId::Home);
        }
        return;
    }
    onBoardClick(p);
}

void GameScreen::select(Position pos) {
    selected_.reset();
    targets_.clear();
    const Piece* piece = board_.getPiece(pos);
    if (piece == nullptr || piece->color() != turn_) return;
    selected_ = pos;
    for (const Move& m : legal_) {
        if (m.from == pos) targets_.push_back(m);
    }
}

void GameScreen::onBoardClick(sf::Vector2f p) {
    if (!humanCanMove()) return;
    const std::optional<Position> sq = squareAt(p);
    if (!sq) return;

    // second click: is it a destination of the selected piece?
    if (selected_) {
        std::vector<Move> options;
        for (const Move& m : legal_) {
            if (m.from == *selected_ && m.to == *sq) options.push_back(m);
        }
        if (!options.empty()) {
            if (options.front().type == MoveType::Promotion) {
                promoOptions_ = options;
                overlay_ = Overlay::Promotion;
            } else {
                playMove(options.front());
            }
            return;
        }
        if (*selected_ == *sq) {                   // clicking the same piece again deselects it
            selected_.reset();
            targets_.clear();
            return;
        }
    }
    select(*sq);
}

void GameScreen::onOverlayClick(sf::Vector2f p) {
    if (overlay_ == Overlay::GameOver) {
        if (overlayButton(0).contains(p)) startNewGame();
        else if (overlayButton(1).contains(p)) overlay_ = Overlay::None;
        else if (overlayButton(2).contains(p)) ctx_.goTo(ScreenId::Home);
    } else if (overlay_ == Overlay::Promotion) {
        const PieceType order[4] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
        for (int i = 0; i < 4; ++i) {
            if (!overlayButton(i).contains(p)) continue;
            for (const Move& m : promoOptions_) {
                if (m.promotion == order[i]) {
                    overlay_ = Overlay::None;
                    playMove(m);
                    return;
                }
            }
        }
        if (!overlayCard().contains(p)) overlay_ = Overlay::None;       // click outside cancels
    } else if (overlay_ == Overlay::Confirm) {
        if (overlayButton(0).contains(p)) {
            overlay_ = Overlay::None;
            if (confirm_ == ConfirmAction::NewGame) startNewGame();
            else ctx_.goTo(ScreenId::Home);
        } else if (overlayButton(1).contains(p) || !overlayCard().contains(p)) {
            overlay_ = Overlay::None;
        }
    }
}

// ----------------------------------------------------------------- drawing

void GameScreen::draw() {
    drawBackground(ctx_.window);
    drawBar(true);
    drawBoard();
    drawBar(false);
    drawSidePanel();
    drawOverlay();
}

void GameScreen::drawBoard() {
    sf::RenderWindow& w = ctx_.window;
    const Fonts& f = ctx_.fonts;
    const Settings& s = ctx_.settings;
    const Theme& theme = themes()[static_cast<std::size_t>(s.themeIndex)];

    drawRoundedRect(w, sf::FloatRect({BOARD_X - 8.f, BOARD_Y - 8.f}, {SQ * 8.f + 16.f, SQ * 8.f + 16.f}), 14.f,
                    sf::Color(255, 255, 255, 22));

    // squares and highlights
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const sf::Vector2f pos = squarePos(r, c);
            sf::RectangleShape sq({SQ, SQ});
            sq.setPosition(pos);
            sq.setFillColor((r + c) % 2 == 0 ? theme.light : theme.dark);
            w.draw(sq);

            const bool isLast = last_ && (last_->from == Position{r, c} || last_->to == Position{r, c});
            const bool isSel = selected_ && *selected_ == Position{r, c};
            if (isSel || isLast) {
                sq.setFillColor(sf::Color(255, 214, 0, isSel ? 140 : 80));
                w.draw(sq);
            }
        }
    }

    // red glow under a king that is in check
    if (status_ == 1 || status_ == 2) {
        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                const Piece* p = board_.getPiece({r, c});
                if (p != nullptr && p->type() == PieceType::King && p->color() == turn_) {
                    const sf::Vector2f pos = squarePos(r, c);
                    drawGlow(w, {pos.x + SQ / 2.f, pos.y + SQ / 2.f}, SQ * 0.62f, sf::Color(235, 40, 40, 215));
                }
            }
        }
    }

    // coordinates along the left and bottom edges
    if (s.showCoords) {
        for (int i = 0; i < 8; ++i) {
            const int rowFrom = flipped_ ? 7 - i : i;
            const int colFrom = flipped_ ? 7 - i : i;
            // rank number on the left column, file letter on the bottom row
            const sf::Vector2f left = squarePos(rowFrom, flipped_ ? 7 : 0);
            drawText(w, f.bold, std::string(1, rankDigit(rowFrom)), 13, {left.x + 7.f, left.y + 11.f},
                     (rowFrom + (flipped_ ? 7 : 0)) % 2 == 0 ? theme.dark : theme.light);
            const sf::Vector2f bottom = squarePos(flipped_ ? 0 : 7, colFrom);
            drawText(w, f.bold, std::string(1, fileLetter(colFrom)), 13, {bottom.x + SQ - 7.f, bottom.y + SQ - 11.f},
                     ((flipped_ ? 0 : 7) + colFrom) % 2 == 0 ? theme.dark : theme.light, Align::Right);
        }
    }

    // pieces (the piece that just moved slides to its square)
    const bool slide = animating();
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = board_.getPiece({r, c});
            if (p == nullptr) continue;
            if (slide && anim_.to == Position{r, c}) continue;
            const sf::Vector2f pos = squarePos(r, c);
            drawPiece(w, f.symbols, p->type(), p->color(), {pos.x + SQ / 2.f, pos.y + SQ / 2.f}, 60);
        }
    }
    if (slide) {
        const Piece* p = board_.getPiece(anim_.to);
        if (p != nullptr) {
            float t = std::min(1.f, animClock_.getElapsedTime().asSeconds() / ANIM_SECONDS);
            t = 1.f - (1.f - t) * (1.f - t);
            const sf::Vector2f a = squarePos(anim_.from.row, anim_.from.col);
            const sf::Vector2f b = squarePos(anim_.to.row, anim_.to.col);
            drawPiece(w, f.symbols, p->type(), p->color(),
                      {a.x + (b.x - a.x) * t + SQ / 2.f, a.y + (b.y - a.y) * t + SQ / 2.f}, 60);
        }
    }

    // legal-move markers: a dot for a quiet move, a ring for a capture
    if (s.showHints && selected_ && !aiPending_) {
        bool done[64] = {};
        for (const Move& m : targets_) {
            const int key = m.to.row * 8 + m.to.col;
            if (done[key]) continue;
            done[key] = true;
            const sf::Vector2f pos = squarePos(m.to.row, m.to.col);
            const sf::Vector2f centre(pos.x + SQ / 2.f, pos.y + SQ / 2.f);
            const bool capture = !board_.isEmpty(m.to) || m.type == MoveType::EnPassant;
            const float radius = capture ? 34.f : 13.f;
            sf::CircleShape marker(radius);
            marker.setOrigin({radius, radius});
            marker.setPosition(centre);
            if (capture) {
                marker.setFillColor(sf::Color::Transparent);
                marker.setOutlineThickness(6.f);
                marker.setOutlineColor(sf::Color(20, 20, 20, 80));
            } else {
                marker.setFillColor(sf::Color(20, 20, 20, 75));
            }
            w.draw(marker);
        }
    }
}

void GameScreen::drawBar(bool top) {
    sf::RenderWindow& w = ctx_.window;
    const Fonts& f = ctx_.fonts;
    const Color color = top ? (flipped_ ? Color::White : Color::Black) : (flipped_ ? Color::Black : Color::White);
    const sf::FloatRect r = barRect(top);
    const bool active = status_ < 2 && turn_ == color;

    drawRoundedRect(w, r, 12.f, active ? sf::Color(242, 184, 75, 30) : sf::Color(255, 255, 255, 0),
                    active ? sf::Color(242, 184, 75, 120) : sf::Color(255, 255, 255, 0), 1.5f);

    const sf::Vector2f c(r.position.x + 28.f, r.position.y + r.size.y / 2.f);
    sf::CircleShape avatar(17.f);
    avatar.setOrigin({17.f, 17.f});
    avatar.setPosition(c);
    avatar.setFillColor(color == Color::White ? sf::Color(58, 64, 84) : sf::Color(205, 210, 224));
    w.draw(avatar);
    drawPiece(w, f.symbols, PieceType::King, color, c, 22);

    drawText(w, f.bold, playerName(color), 16, {r.position.x + 56.f, c.y}, palette::text);

    // pieces this side has captured are the opponent's missing pieces
    const Material mat = material();
    const Color opp = other(color);
    float x = r.position.x + 300.f;
    for (int i = 0; i < 5; ++i) {
        for (int n = 0; n < mat.captured[static_cast<int>(opp)][i]; ++n) {
            drawPiece(w, f.symbols, SHOW_ORDER[i], opp, {x, c.y}, 20);
            x += 14.f;
        }
        x += 4.f;
    }
    const int lead = color == Color::White ? mat.lead : -mat.lead;
    if (lead > 0) {
        drawText(w, f.bold, "+" + std::to_string(lead), 15, {r.position.x + r.size.x - 14.f, c.y}, palette::muted,
                 Align::Right);
    }
}

void GameScreen::drawStatusText() {
    sf::RenderWindow& w = ctx_.window;
    const Fonts& f = ctx_.fonts;
    const sf::FloatRect r = statusRect();

    std::string mainText;
    std::string sub;
    if (status_ == 2) {
        mainText = "Checkmate";
        if (vsAI_) sub = (other(turn_) == human_) ? "You win!" : "The computer wins.";
        else sub = std::string(turn_ == Color::White ? "Black" : "White") + " wins.";
    } else if (status_ == 3) {
        mainText = "Stalemate";
        sub = "The game is a draw.";
    } else if (aiPending_) {
        mainText = "Computer is thinking\xE2\x80\xA6";
    } else {
        if (vsAI_) {
            mainText = turn_ == human_ ? "Your move" : "Computer's move";
            sub = std::string("You are playing ") + (human_ == Color::White ? "White." : "Black.");
        } else {
            mainText = std::string(turn_ == Color::White ? "White" : "Black") + " to move";
        }
        if (status_ == 1) sub = "Check!";
    }

    sf::CircleShape dot(11.f);
    dot.setOrigin({11.f, 11.f});
    dot.setPosition({r.position.x + 32.f, r.position.y + r.size.y / 2.f});
    dot.setFillColor(turn_ == Color::White ? sf::Color::White : sf::Color(20, 20, 20));
    dot.setOutlineThickness(2.f);
    dot.setOutlineColor(turn_ == Color::White ? sf::Color(120, 120, 120) : sf::Color(150, 150, 150));
    w.draw(dot);

    drawText(w, f.title, mainText, 22, {r.position.x + 58.f, r.position.y + 38.f}, palette::text);
    drawText(w, f.text, sub, 15, {r.position.x + 58.f, r.position.y + 68.f},
             status_ == 1 ? sf::Color(255, 130, 120) : palette::muted);
}

void GameScreen::drawSidePanel() {
    sf::RenderWindow& w = ctx_.window;
    const Fonts& f = ctx_.fonts;

    drawCard(w, statusRect());
    drawStatusText();

    // move list
    const sf::FloatRect mr = movesRect();
    drawCard(w, mr);
    drawText(w, f.bold, "MOVES", 13, {mr.position.x + 18.f, mr.position.y + 24.f}, palette::muted);
    if (history_.empty()) {
        drawText(w, f.text, "No moves yet.", 15, {mr.position.x + 18.f, mr.position.y + 66.f}, palette::muted);
    } else {
        const int rows = static_cast<int>((history_.size() + 1) / 2);
        const int first = std::max(0, rows - VISIBLE_ROWS - scrollRows_);
        for (int row = first; row < std::min(rows, first + VISIBLE_ROWS); ++row) {
            const float y = mr.position.y + 62.f + static_cast<float>(row - first) * 26.f;
            if (row % 2 == 0) {
                drawRoundedRect(w, sf::FloatRect({mr.position.x + 10.f, y - 12.f}, {mr.size.x - 20.f, 26.f}), 6.f,
                                sf::Color(255, 255, 255, 10));
            }
            drawText(w, f.text, std::to_string(row + 1) + ".", 15, {mr.position.x + 18.f, y}, palette::muted);
            drawText(w, f.text, history_[static_cast<std::size_t>(row) * 2].san, 16, {mr.position.x + 62.f, y},
                     palette::text);
            const std::size_t black = static_cast<std::size_t>(row) * 2 + 1;
            if (black < history_.size()) {
                drawText(w, f.text, history_[black].san, 16, {mr.position.x + 160.f, y}, palette::text);
            }
        }
    }

    // buttons
    const char* labels[4] = {"Undo", "Flip board", "New game", "Menu"};
    for (int i = 0; i < 4; ++i) {
        const bool enabled = (i != 0) || undoSteps() > 0;
        drawButton(w, f, buttonRect(i), labels[i], overlay_ == Overlay::None && buttonRect(i).contains(mouse_),
                   false, enabled);
    }
}

void GameScreen::drawOverlay() {
    if (overlay_ == Overlay::None) return;
    sf::RenderWindow& w = ctx_.window;
    const Fonts& f = ctx_.fonts;

    sf::RectangleShape dim({WIDTH, HEIGHT});
    dim.setFillColor(sf::Color(6, 8, 14, 175));
    w.draw(dim);

    const sf::FloatRect card = overlayCard();
    drawRoundedRect(w, card, 22.f, sf::Color(26, 31, 45), palette::border, 1.5f);
    const float cx = card.position.x + card.size.x / 2.f;
    const float y = card.position.y;

    if (overlay_ == Overlay::GameOver) {
        std::string big;
        std::string text;
        if (status_ == 3) {
            big = "Draw";
            text = "Stalemate: no legal move, but not in check.";
        } else if (vsAI_) {
            const bool won = other(turn_) == human_;
            big = won ? "You win!" : "You lose";
            text = won ? "Checkmate. Well played!" : "Checkmate. The computer wins this one.";
        } else {
            big = std::string(turn_ == Color::White ? "Black" : "White") + " wins";
            text = "Checkmate.";
        }
        drawText(w, f.bold, "GAME OVER", 13, {cx, y + 40.f}, palette::muted, Align::Center);
        drawText(w, f.title, big, 38, {cx, y + 90.f}, palette::text, Align::Center);
        drawText(w, f.text, text, 16, {cx, y + 134.f}, palette::muted, Align::Center);
        drawButton(w, f, overlayButton(0), "Play again", overlayButton(0).contains(mouse_), true);
        drawButton(w, f, overlayButton(1), "View board", overlayButton(1).contains(mouse_));
        drawButton(w, f, overlayButton(2), "Menu", overlayButton(2).contains(mouse_));
    } else if (overlay_ == Overlay::Promotion) {
        drawText(w, f.bold, "PROMOTE PAWN TO", 13, {cx, y + 40.f}, palette::muted, Align::Center);
        const PieceType order[4] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
        const Theme& theme = themes()[static_cast<std::size_t>(ctx_.settings.themeIndex)];
        for (int i = 0; i < 4; ++i) {
            const sf::FloatRect b = overlayButton(i);
            const bool hover = b.contains(mouse_);
            drawRoundedRect(w, b, 14.f, turn_ == Color::White ? theme.dark : theme.light,
                            hover ? palette::accent : sf::Color::Transparent, 3.f);
            drawPiece(w, f.symbols, order[i], turn_, {b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f}, 52);
        }
    } else {
        drawText(w, f.title, confirm_ == ConfirmAction::NewGame ? "Start a new game?" : "Leave this game?", 26,
                 {cx, y + 48.f}, palette::text, Align::Center);
        drawText(w, f.text, "The current game will be lost.", 15, {cx, y + 82.f}, palette::muted, Align::Center);
        drawButton(w, f, overlayButton(0), "Yes", overlayButton(0).contains(mouse_), true);
        drawButton(w, f, overlayButton(1), "Cancel", overlayButton(1).contains(mouse_));
    }
}

}  // namespace gui
