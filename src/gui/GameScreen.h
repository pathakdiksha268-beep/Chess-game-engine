#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Gui.h"
#include "engine/Board.h"
#include "analyzer/MinimaxAI.h"

namespace gui {

// Screen 3: the game itself. It only uses the public interface of Board and MinimaxAI.
class GameScreen : public Screen {
public:
    explicit GameScreen(Context& ctx);

    void handleEvent(const sf::Event& event) override;
    void update() override;
    void draw() override;

private:
    enum class Overlay { None, Promotion, GameOver, Confirm };
    enum class ConfirmAction { NewGame, Menu };
    struct Entry { Move move; std::string san; };
    struct Material { int captured[2][5]; int lead; };   // captured[colour][q r b n p], lead = White minus Black

    // ---- layout (window units) ----
    static constexpr float BOARD_X = 40.f;
    static constexpr float BOARD_Y = 64.f;
    static constexpr float SQ = 80.f;

    sf::FloatRect barRect(bool top) const;
    sf::FloatRect statusRect() const { return sf::FloatRect({720.f, 64.f}, {280.f, 100.f}); }
    sf::FloatRect movesRect() const { return sf::FloatRect({720.f, 180.f}, {280.f, 330.f}); }
    sf::FloatRect buttonRect(int index) const;
    sf::FloatRect overlayCard() const;
    sf::FloatRect overlayButton(int index) const;

    sf::Vector2f squarePos(int row, int col) const;
    std::optional<Position> squareAt(sf::Vector2f p) const;

    // ---- game logic ----
    void startNewGame();
    void refresh();                       // legal moves and check / mate status
    void playMove(const Move& move);
    void maybeStartAi();
    bool humanCanMove() const;
    int undoSteps() const;
    void undo();
    Material material() const;
    std::string sanBase(const Move& m) const;
    std::string playerName(Color c) const;

    // ---- input ----
    void onClick(sf::Vector2f p);
    void onBoardClick(sf::Vector2f p);
    void onOverlayClick(sf::Vector2f p);
    void select(Position pos);

    // ---- drawing ----
    void drawBoard();
    void drawBar(bool top);
    void drawSidePanel();
    void drawOverlay();
    void drawStatusText();
    bool animating() const;

    Board board_;
    MinimaxAI ai_;
    Color turn_ = Color::White;
    Color human_ = Color::White;
    bool vsAI_ = true;
    bool flipped_ = false;
    int status_ = 0;                      // 0 normal, 1 check, 2 checkmate, 3 stalemate
    std::vector<Move> legal_;
    std::optional<Position> selected_;
    std::vector<Move> targets_;
    std::optional<Move> last_;
    std::vector<Entry> history_;

    bool aiPending_ = false;
    int aiDelay_ = 0;
    int overlayDelay_ = 0;
    int scrollRows_ = 0;

    Overlay overlay_ = Overlay::None;
    ConfirmAction confirm_ = ConfirmAction::NewGame;
    std::vector<Move> promoOptions_;

    struct Animation { Position from; Position to; bool active = false; };
    Animation anim_;
    sf::Clock animClock_;

    sf::Vector2f mouse_{-1.f, -1.f};
};

}  // namespace gui
