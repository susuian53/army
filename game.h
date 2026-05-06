#pragma once
#include <vector>

enum class Side { SIDE_RED, SIDE_BLUE, SIDE_NONE };
enum class PieceType { 
    PT_COMMANDER, PT_CORPS, PT_DIVISION, PT_BRIGADE, PT_REGIMENT, 
    PT_BATTALION, PT_COMPANY, PT_PLATOON, PT_ENGINEER, 
    PT_BOMB, PT_MINE, PT_FLAG, PT_EMPTY 
};

struct Piece {
    PieceType type;
    Side side;
    bool isVisible;
    Piece(PieceType t = PieceType::PT_EMPTY, Side s = Side::SIDE_NONE) : type(t), side(s), isVisible(false) {}
};

struct Pos {
    int r, c;
    bool operator==(const Pos& other) const { return r == other.r && c == other.c; }
    bool operator!=(const Pos& other) const { return !(*this == other); }
};

class GameLogic {
public:
    static const int ROWS = 12;
    static const int COLS = 5;
    
    GameLogic();
    void initBoard();
    
    Piece getPiece(Pos p) const;
    Side getCurrentTurn() const { return currentTurn; }
    void switchTurn();
    
    bool flipPiece(Pos p);
    int movePiece(Pos from, Pos to); // Returns 0: failed, 1: moved to empty, 2: captured/clashed
    bool isValidMove(Pos from, Pos to) const;
    int checkWinner();
    void aiMove();

    bool isSafeZone(Pos p) const;
    bool isHeadquarters(Pos p) const;
    bool isRailway(Pos p) const;
    bool isWithinBoard(Pos p) const;
    bool areConnected(Pos from, Pos to) const;

private:
    Piece board[ROWS][COLS];
    Side currentTurn;
    
    int getRank(PieceType t) const;
    bool canEat(Piece attacker, Piece target) const;
};
