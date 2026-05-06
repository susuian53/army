#include "draw.h"

void Drawer::drawBoard(const GameLogic& game, Pos selectedPos) {
    // 1. Background
    setbkcolor(RGB(220, 200, 150));
    cleardevice();

    // 2. Grid lines (Highways)
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 1);
    
    // Horizontal lines
    for (int r = 0; r < GameLogic::ROWS; ++r) {
        if (r == 5 || r == 6) continue;
        line(OFFSET_X, OFFSET_Y + r * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + r * GRID_H);
    }
    
    // Vertical lines
    for (int c = 0; c < GameLogic::COLS; ++c) {
        line(OFFSET_X + c * GRID_W, OFFSET_Y, OFFSET_X + c * GRID_W, OFFSET_Y + 5 * GRID_H);
        line(OFFSET_X + c * GRID_W, OFFSET_Y + 6 * GRID_H, OFFSET_X + c * GRID_W, OFFSET_Y + 11 * GRID_H);
        // Add vertical paths across the river for columns 0, 2, 4
        if (c == 0 || c == 2 || c == 4) {
            line(OFFSET_X + c * GRID_W, OFFSET_Y + 5 * GRID_H, OFFSET_X + c * GRID_W, OFFSET_Y + 6 * GRID_H);
        }
    }

    // 3. Boundary (River)
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);
    line(OFFSET_X, OFFSET_Y + 5 * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + 5 * GRID_H);
    line(OFFSET_X, OFFSET_Y + 6 * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + 6 * GRID_H);
    
    settextcolor(BLACK);
    settextstyle(42, 0, L"\u5fae\u8f6f\u96c5\u9ed1"); // Larger font for river
    outtextxy(OFFSET_X + 80, OFFSET_Y + 5 * GRID_H + 12, L"\u695a \u6cb3");
    outtextxy(OFFSET_X + 320, OFFSET_Y + 5 * GRID_H + 12, L"\u6c49 \u754c");

    // 4. Railways (Thick grey lines)
    setlinecolor(RGB(80, 80, 80));
    setlinestyle(PS_SOLID, 8); // Thicker railways
    // Horizontal railways
    int railwayRows[] = {1, 5, 6, 10};
    for (int r : railwayRows) {
        line(OFFSET_X, OFFSET_Y + r * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + r * GRID_H);
    }
    // Vertical railways
    line(OFFSET_X, OFFSET_Y + 1 * GRID_H, OFFSET_X, OFFSET_Y + 10 * GRID_H);
    line(OFFSET_X + 4 * GRID_W, OFFSET_Y + 1 * GRID_H, OFFSET_X + 4 * GRID_W, OFFSET_Y + 10 * GRID_H);

    // 5. Draw connections for Safe Zones (行营)
    setlinestyle(PS_SOLID, 1);
    setlinecolor(BLACK);
    struct Connection { int r1, c1, r2, c2; };
    std::vector<Connection> connections = {
        {1,0, 2,1}, {1,2, 2,1}, {3,0, 2,1}, {3,2, 2,1}, {2,0, 2,1}, {2,2, 2,1}, {1,1, 2,1}, {3,1, 2,1},
        {1,2, 2,3}, {1,4, 2,3}, {3,2, 2,3}, {3,4, 2,3}, {2,2, 2,3}, {2,4, 2,3}, {1,3, 2,3}, {3,3, 2,3},
        {2,1, 3,2}, {2,3, 3,2}, {4,1, 3,2}, {4,3, 3,2}, {3,1, 3,2}, {3,3, 3,2}, {2,2, 3,2}, {4,2, 3,2},
        {3,0, 4,1}, {3,2, 4,1}, {5,0, 4,1}, {5,2, 4,1}, {4,0, 4,1}, {4,2, 4,1}, {3,1, 4,1}, {5,1, 4,1},
        {3,2, 4,3}, {3,4, 4,3}, {5,2, 4,3}, {5,4, 4,3}, {4,2, 4,3}, {4,4, 4,3}, {3,3, 4,3}, {5,3, 4,3},
        
        {6,0, 7,1}, {6,2, 7,1}, {8,0, 7,1}, {8,2, 7,1}, {7,0, 7,1}, {7,2, 7,1}, {6,1, 7,1}, {8,1, 7,1},
        {6,2, 7,3}, {6,4, 7,3}, {8,2, 7,3}, {8,4, 7,3}, {7,2, 7,3}, {7,4, 7,3}, {6,3, 7,3}, {8,3, 7,3},
        {7,1, 8,2}, {7,3, 8,2}, {9,1, 8,2}, {9,3, 8,2}, {8,1, 8,2}, {8,3, 8,2}, {7,2, 8,2}, {9,2, 8,2},
        {8,0, 9,1}, {8,2, 9,1}, {10,0, 9,1}, {10,2, 9,1}, {9,0, 9,1}, {9,2, 9,1}, {8,1, 9,1}, {10,1, 9,1},
        {8,2, 9,3}, {8,4, 9,3}, {10,2, 9,3}, {10,4, 9,3}, {9,2, 9,3}, {9,4, 9,3}, {8,3, 9,3}, {10,3, 9,3}
    };
    for (auto& c : connections) {
        line(OFFSET_X + c.c1 * GRID_W, OFFSET_Y + c.r1 * GRID_H, OFFSET_X + c.c2 * GRID_W, OFFSET_Y + c.r2 * GRID_H);
    }

    // 6. Draw Safe Zones and Headquarters (Without text as requested)
    for (int r = 0; r < GameLogic::ROWS; ++r) {
        for (int c = 0; c < GameLogic::COLS; ++c) {
            Pos p = {r, c};
            int x = OFFSET_X + c * GRID_W;
            int y = OFFSET_Y + r * GRID_H;
            if (game.isSafeZone(p)) {
                setfillcolor(RGB(240, 230, 180));
                fillcircle(x, y, 27); // Enlarged circle
            } else if (game.isHeadquarters(p)) {
                setfillcolor(RGB(180, 140, 100));
                fillrectangle(x - 52, y - 27, x + 52, y + 27); // Enlarged rectangle
            } else {
                setfillcolor(BLACK);
                solidcircle(x, y, 5); // Larger station dot
            }
        }
    }

    // 7. Draw Pieces
    for (int r = 0; r < GameLogic::ROWS; ++r) {
        for (int c = 0; c < GameLogic::COLS; ++c) {
            Pos p = {r, c};
            Piece piece = game.getPiece(p);
            if (piece.type != PieceType::PT_EMPTY) {
                drawPiece(p, piece, (p == selectedPos));
            }
        }
    }
}

void Drawer::drawPiece(Pos p, Piece piece, bool isSelected) {
    int x = OFFSET_X + p.c * GRID_W;
    int y = OFFSET_Y + p.r * GRID_H;

    // 1. Draw back side if hidden
    if (!piece.isVisible) {
        setfillcolor(RGB(120, 120, 120)); // Gray back
        setlinecolor(BLACK);
        fillroundrect(x - 45, y - 25, x + 45, y + 25, 8, 8);
        return;
    }

    // 2. Determine color for visible piece
    if (piece.side == Side::SIDE_RED) {
        setfillcolor(RGB(255, 100, 100));
    } else {
        setfillcolor(RGB(100, 100, 255));
    }

    // 3. Highlight if selected
    if (isSelected) {
        setlinecolor(YELLOW);
        setlinestyle(PS_SOLID, 3);
    } else {
        setlinecolor(BLACK);
        setlinestyle(PS_SOLID, 1);
    }

    // 4. Draw piece body
    fillroundrect(x - 45, y - 25, x + 45, y + 25, 8, 8);
    setlinestyle(PS_SOLID, 1);

    // 5. Draw piece text
    const wchar_t* text = L"";
    switch (piece.type) {
        case PieceType::PT_COMMANDER: text = L"\u53f8"; break;
        case PieceType::PT_CORPS:     text = L"\u519b"; break;
        case PieceType::PT_DIVISION:  text = L"\u5e08"; break;
        case PieceType::PT_BRIGADE:   text = L"\u65c5"; break;
        case PieceType::PT_REGIMENT:  text = L"\u56e2"; break;
        case PieceType::PT_BATTALION: text = L"\u8425"; break;
        case PieceType::PT_COMPANY:   text = L"\u8fde"; break;
        case PieceType::PT_PLATOON:   text = L"\u6392"; break;
        case PieceType::PT_ENGINEER:  text = L"\u5de5"; break;
        case PieceType::PT_BOMB:       text = L"\u70b8"; break;
        case PieceType::PT_MINE:       text = L"\u96f7"; break;
        case PieceType::PT_FLAG:       text = L"\u65d7"; break;
        default: break;
    }

    settextcolor(WHITE);
    settextstyle(28, 0, L"\u5fae\u8f6f\u96c5\u9ed1");
    setbkmode(TRANSPARENT);
    int tw = textwidth(text);
    int th = textheight(text);
    outtextxy(x - tw / 2, y - th / 2, text);
}

void Drawer::drawUI(const GameLogic& game, int winner) {
    // 绘制游戏状态
    settextcolor(BLACK);
    settextstyle(30, 0, L"\u5fae\u8f6f\u96c5\u9ed1");
    
    std::wstring turnStr;
    if (winner == 0) {
        if (game.getCurrentTurn() == Side::SIDE_RED) {
            settextcolor(RGB(200, 0, 0));
            turnStr = L"\u5f53\u524d\u56de\u5408\uff1a\u73a9\u5bb6 (\u7ea2\u65b9)";
        } else {
            settextcolor(RGB(0, 0, 200));
            turnStr = L"\u5f53\u524d\u56de\u5408\uff1aAI (\u84dd\u65b9)";
        }
    } else {
        turnStr = L"\u6e38\u620f\u7ed3\u675f";
    }
    
    outtextxy(20, 20, turnStr.c_str());

    // 绘制规则说明
    drawRules();

    if (winner != 0) {
        settextcolor(RED);
        settextstyle(60, 0, L"\u5fae\u8f6f\u96c5\u9ed1");
        const wchar_t* winStr = (winner == 1) ? L"\u7ea2\u65b9\u80dc\u5229\uff01" : L"\u84dd\u65b9\u80dc\u5229\uff01";
        
        int tw = textwidth(winStr);
        int th = textheight(winStr);
        outtextxy((WIN_WIDTH - tw) / 2, (WIN_HEIGHT - th) / 2, winStr);
    }
}

void Drawer::drawRules() {
    int startX = OFFSET_X + (GameLogic::COLS - 1) * GRID_W + 40;
    int startY = 100;
    
    settextcolor(RGB(50, 50, 50));
    settextstyle(24, 0, L"\u5fae\u8f6f\u96c5\u9ed1", 0, 0, FW_BOLD, false, false, false);
    outtextxy(startX, startY, L"\u6e38\u620f\u89c4\u5219\u8bf4\u660e\uff1a");
    
    settextstyle(18, 0, L"\u5fae\u8f6f\u96c5\u9ed1");
    int lineHeight = 28;
    int curY = startY + 40;
    
    const wchar_t* rules[] = {
        L"1. \u7ffb\u68cb\u6ā1\u5f0f\uff1a\u521d\u59cb\u68cb\u5b50\u80cc\u9762\u671d\u4e0a",
        L"2. \u53cc\u51fb\u7ffb\u724c\uff0c\u5355\u51fb\u9009\u62e9/\u79fb\u52a8",
        L"3. \u884c\u8425\u89c4\u5219\uff1a",
        L"   - \u884c\u8425\u5185\u4e0d\u53ef\u88ab\u653b\u51fb",
        L"   - \u5730\u96f7\u3001\u70b8\u5f39\u4e0d\u53ef\u8fdb\u5165\u884c\u8425",
        L"   - \u884c\u8425\u5468\u56f4\u652f\u6301\u659c\u5411\u79fb\u52a8",
        L"4. \u68cb\u5b50\u9650\u5236\uff1a",
        L"   - \u519b\u65d7\u3001\u5730\u96f7\u4e0d\u53ef\u79fb\u52a8",
        L"   - \u70b8\u5f39\u53ef\u4e0e\u4efb\u4f55\u68cb\u5b50\u540c\u5f52\u4e8e\u5c3d",
        L"   - \u5730\u96f7\u4ec5\u5de5\u5175\u53ef\u62c6\u9664",
        L"5. \u94c1\u8def\u89c4\u5219\uff1a",
        L"   - \u94c1\u8def\u4e0a\u53ef\u76f4\u7ebf\u8繝\u8ddd\u79fb\u52a8",
        L"6. \u80dc\u8d1f\u5224\u5b9a\uff1a",
        L"   - \u5403\u6389\u519b\u65d7\u6216\u5bf9\u65b9\u65e0\u6cd5\u79fb\u52a8",
        L"   - \u9700\u62c6\u9664\u6240\u6709\u5730\u96f7\u65b9\u53ef\u5403\u519b\u65d7"
    };
    
    for (const wchar_t* rule : rules) {
        outtextxy(startX, curY, rule);
        curY += lineHeight;
    }
}
Pos Drawer::screenToBoard(int x, int y) {
    int c = (x - OFFSET_X + GRID_W / 2) / GRID_W;
    int r = (y - OFFSET_Y + GRID_H / 2) / GRID_H;
    return {r, c};
}
void Drawer::showMessage(const std::string& msg) {}
