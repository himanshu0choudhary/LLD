#include <bits/stdc++.h>
using namespace std;

/* ================= ENUMS ================= */

enum class Color
{
    WHITE,
    BLACK
};

Color opposite(Color c)
{
    return (c == Color::WHITE) ? Color::BLACK : Color::WHITE;
}

/* ================= FORWARD DECLARATIONS ================= */

class Board;
class Game;
class Move;
class GameContext;

/* ================= POSITION ================= */

struct Position
{
    int r, c;
    Position(int r = 0, int c = 0) : r(r), c(c) {}

    bool operator==(const Position &other) const
    {
        return r == other.r && c == other.c;
    }
};

/* ================= MOVE ================= */

struct Move
{
    Position from;
    Position to;
    bool isEnPassant = false;

    Move(Position f, Position t) : from(f), to(t) {}
};

/* ================= CONTEXT ================= */

struct GameContext
{
    Move *lastMove = nullptr;
};

/* ================= STRATEGY ================= */

class MoveStrategy
{
public:
    virtual bool isValidMove(Move &, Board &, GameContext &, Color) = 0;
    virtual ~MoveStrategy() = default;
};

/* ================= PIECE ================= */

class Piece
{
public:
    Color color;
    Position pos;
    unique_ptr<MoveStrategy> strategy;

    Piece(Color c, Position p, unique_ptr<MoveStrategy> s)
        : color(c), pos(p), strategy(move(s)) {}

    virtual char symbol() = 0;

    bool isValidMove(Move &m, Board &b, GameContext &ctx)
    {
        return strategy->isValidMove(m, b, ctx, color);
    }

    virtual ~Piece() = default;
};

/* ================= BOARD ================= */

class Board
{
public:
    vector<vector<Piece *>> grid;

    Board()
    {
        grid.resize(8, vector<Piece *>(8, nullptr));
    }

    bool inBounds(Position p)
    {
        return p.r >= 0 && p.r < 8 && p.c >= 0 && p.c < 8;
    }

    Piece *get(Position p)
    {
        return grid[p.r][p.c];
    }

    void set(Position p, Piece *piece)
    {
        grid[p.r][p.c] = piece;
        if (piece)
            piece->pos = p;
    }

    bool isPathClear(Position from, Position to)
    {
        int dr = (to.r - from.r) == 0 ? 0 : (to.r - from.r) / abs(to.r - from.r);
        int dc = (to.c - from.c) == 0 ? 0 : (to.c - from.c) / abs(to.c - from.c);

        Position cur = {from.r + dr, from.c + dc};
        while (!(cur == to))
        {
            if (get(cur) != nullptr)
                return false;
            cur.r += dr;
            cur.c += dc;
        }
        return true;
    }

    void print()
    {
        cout << "\n";
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (grid[i][j])
                    cout << grid[i][j]->symbol() << " ";
                else
                    cout << ". ";
            }
            cout << "\n";
        }
        cout << "\n";
    }
};

/* ================= STRATEGIES ================= */

class RookStrategy : public MoveStrategy
{
public:
    bool isValidMove(Move &m, Board &b, GameContext &, Color color) override
    {
        if (m.from.r != m.to.r && m.from.c != m.to.c)
            return false;
        if (!b.isPathClear(m.from, m.to))
            return false;
        Piece *target = b.get(m.to);
        return !target || target->color != color;
    }
};

class BishopStrategy : public MoveStrategy
{
public:
    bool isValidMove(Move &m, Board &b, GameContext &, Color color) override
    {
        if (abs(m.from.r - m.to.r) != abs(m.from.c - m.to.c))
            return false;
        if (!b.isPathClear(m.from, m.to))
            return false;
        Piece *target = b.get(m.to);
        return !target || target->color != color;
    }
};

class KnightStrategy : public MoveStrategy
{
public:
    bool isValidMove(Move &m, Board &b, GameContext &, Color color) override
    {
        int dr = abs(m.from.r - m.to.r);
        int dc = abs(m.from.c - m.to.c);
        if (!((dr == 2 && dc == 1) || (dr == 1 && dc == 2)))
            return false;
        Piece *target = b.get(m.to);
        return !target || target->color != color;
    }
};

class KingStrategy : public MoveStrategy
{
public:
    bool isValidMove(Move &m, Board &b, GameContext &, Color color) override
    {
        int dr = abs(m.from.r - m.to.r);
        int dc = abs(m.from.c - m.to.c);
        if (dr <= 1 && dc <= 1)
        {
            Piece *target = b.get(m.to);
            return !target || target->color != color;
        }
        return false;
    }
};

class PawnStrategy : public MoveStrategy
{
public:
    bool isValidMove(Move &m, Board &b, GameContext &ctx, Color color) override
    {
        int dir = (color == Color::WHITE) ? -1 : 1;

        // Forward
        if (m.from.c == m.to.c)
        {
            if (m.to.r == m.from.r + dir && !b.get(m.to))
                return true;

            // Double move
            if ((color == Color::WHITE && m.from.r == 6) ||
                (color == Color::BLACK && m.from.r == 1))
            {
                if (m.to.r == m.from.r + 2 * dir &&
                    !b.get(m.to) &&
                    !b.get({m.from.r + dir, m.from.c}))
                    return true;
            }
        }

        // Capture
        if (abs(m.from.c - m.to.c) == 1 &&
            m.to.r == m.from.r + dir)
        {

            Piece *target = b.get(m.to);
            if (target && target->color != color)
                return true;

            // En-passant
            if (!target && ctx.lastMove != nullptr)
            {
                Move *last = ctx.lastMove;
                Piece *lastPiece = b.get(last->to);

                if (lastPiece &&
                    dynamic_cast<Pawn *>(lastPiece) &&
                    abs(last->from.r - last->to.r) == 2 &&
                    last->to.r == m.from.r &&
                    last->to.c == m.to.c)
                {
                    m.isEnPassant = true;
                    return true;
                }
            }
        }

        return false;
    }
};

/* ================= PIECES ================= */

class Rook : public Piece
{
public:
    Rook(Color c, Position p)
        : Piece(c, p, make_unique<RookStrategy>()) {}
    char symbol() override { return (color == Color::WHITE) ? 'R' : 'r'; }
};

class Bishop : public Piece
{
public:
    Bishop(Color c, Position p)
        : Piece(c, p, make_unique<BishopStrategy>()) {}
    char symbol() override { return (color == Color::WHITE) ? 'B' : 'b'; }
};

class Knight : public Piece
{
public:
    Knight(Color c, Position p)
        : Piece(c, p, make_unique<KnightStrategy>()) {}
    char symbol() override { return (color == Color::WHITE) ? 'N' : 'n'; }
};

class King : public Piece
{
public:
    King(Color c, Position p)
        : Piece(c, p, make_unique<KingStrategy>()) {}
    char symbol() override { return (color == Color::WHITE) ? 'K' : 'k'; }
};

class Pawn : public Piece
{
public:
    Pawn(Color c, Position p)
        : Piece(c, p, make_unique<PawnStrategy>()) {}
    char symbol() override { return (color == Color::WHITE) ? 'P' : 'p'; }
};

/* ================= COMMAND ================= */

class Command
{
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~Command() = default;
};

class MoveCommand : public Command
{
    Board &board;
    GameContext &ctx;
    Move move;
    Piece *captured = nullptr;
    Move *previousLastMove = nullptr;

public:
    MoveCommand(Board &b, GameContext &c, Move m)
        : board(b), ctx(c), move(m) {}

    void execute() override
    {
        previousLastMove = ctx.lastMove;

        Piece *piece = board.get(move.from);

        if (move.isEnPassant)
        {
            Position capPos(move.from.r, move.to.c);
            captured = board.get(capPos);
            board.set(capPos, nullptr);
        }
        else
        {
            captured = board.get(move.to);
        }

        board.set(move.to, piece);
        board.set(move.from, nullptr);

        ctx.lastMove = new Move(move);
    }

    void undo() override
    {
        Piece *piece = board.get(move.to);
        board.set(move.from, piece);
        board.set(move.to, captured);

        delete ctx.lastMove;
        ctx.lastMove = previousLastMove;
    }
};

/* ================= STATE ================= */

class GameState
{
public:
    virtual void handleMove(Game &, Move &) = 0;
    virtual ~GameState() = default;
};

class Game;

/* ================= GAME ================= */

class Game
{
public:
    Board board;
    GameContext ctx;
    Color currentTurn = Color::WHITE;
    stack<unique_ptr<Command>> history;
    unique_ptr<GameState> state;

    Game();
    void makeMove(Move m);
    void undo();
    void switchTurn() { currentTurn = opposite(currentTurn); }
};

class OngoingState : public GameState
{
public:
    void handleMove(Game &g, Move &m) override
    {
        Piece *piece = g.board.get(m.from);

        if (!piece || piece->color != g.currentTurn)
            throw runtime_error("Invalid turn");

        if (!piece->isValidMove(m, g.board, g.ctx))
            throw runtime_error("Invalid move");

        auto cmd = make_unique<MoveCommand>(g.board, g.ctx, m);
        cmd->execute();
        g.history.push(move(cmd));

        g.switchTurn();
    }
};

Game::Game()
{
    state = make_unique<OngoingState>();

    board.set({6, 4}, new Pawn(Color::WHITE, {6, 4}));
    board.set({1, 3}, new Pawn(Color::BLACK, {1, 3}));
}

void Game::makeMove(Move m)
{
    state->handleMove(*this, m);
}

void Game::undo()
{
    if (history.empty())
        return;

    history.top()->undo();
    history.pop();
    switchTurn();
}

/* ================= MAIN ================= */

int main()
{
    Game game;

    cout << "Initial Board:";
    game.board.print();

    game.makeMove(Move({6, 4}, {4, 4}));
    cout << "After White Pawn Move:";
    game.board.print();

    game.makeMove(Move({1, 3}, {3, 3}));
    cout << "After Black Pawn Move:";
    game.board.print();

    Move enPassant({4, 4}, {3, 3});
    game.makeMove(enPassant);
    cout << "After En-Passant:";
    game.board.print();

    game.undo();
    cout << "After Undo:";
    game.board.print();

    return 0;
}