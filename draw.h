#pragma once
#include "game.h"
#include "easyx_config.h"
#include <string>

class Drawer {
public:
    static const int WIN_WIDTH = 1080;
    static const int WIN_HEIGHT = 810;
    static const int GRID_W = 120;
    static const int GRID_H = 68;
    static const int OFFSET_X = 300;
    static const int OFFSET_Y = 33;

    void drawBoard(const GameLogic& game, Pos selectedPos);
    void drawPiece(Pos p, Piece piece, bool isSelected);
    void drawUI(const GameLogic& game, int winner);
    void drawRules();
    Pos screenToBoard(int x, int y);
    void showMessage(const std::string& msg);
};
