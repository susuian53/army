#include "draw.h"

// 绘制整个棋盘
void Drawer::drawBoard(const GameLogic& game, Pos selectedPos) {
    // 1. 背景绘制
    setbkcolor(RGB(220, 200, 150)); // 浅木色背景
    cleardevice();

    // 2. 绘制普通公路格线
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 1);
    
    // 绘制横线
    for (int r = 0; r < GameLogic::ROWS; ++r) {
        if (r == 5 || r == 6) continue; // 楚河汉界处不画横线
        line(OFFSET_X, OFFSET_Y + r * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + r * GRID_H);
    }
    
    // 绘制竖线
    for (int c = 0; c < GameLogic::COLS; ++c) {
        line(OFFSET_X + c * GRID_W, OFFSET_Y, OFFSET_X + c * GRID_W, OFFSET_Y + 5 * GRID_H);
        line(OFFSET_X + c * GRID_W, OFFSET_Y + 6 * GRID_H, OFFSET_X + c * GRID_W, OFFSET_Y + 11 * GRID_H);
        // 在 0, 2, 4 列绘制跨河连接线
        if (c == 0 || c == 2 || c == 4) {
            line(OFFSET_X + c * GRID_W, OFFSET_Y + 5 * GRID_H, OFFSET_X + c * GRID_W, OFFSET_Y + 6 * GRID_H);
        }
    }

    // 3. 绘制楚河汉界（边界）
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);
    line(OFFSET_X, OFFSET_Y + 5 * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + 5 * GRID_H);
    line(OFFSET_X, OFFSET_Y + 6 * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + 6 * GRID_H);
    
    settextcolor(BLACK);
    settextstyle(42, 0, L"微软雅黑"); // 较大的河界字体
    outtextxy(OFFSET_X + 80, OFFSET_Y + 5 * GRID_H + 12, L"楚 河");
    outtextxy(OFFSET_X + 320, OFFSET_Y + 5 * GRID_H + 12, L"汉 界");

    // 4. 绘制铁路（粗灰线）
    setlinecolor(RGB(80, 80, 80));
    setlinestyle(PS_SOLID, 8); // 铁路加粗
    // 横向铁路
    int railwayRows[] = {1, 5, 6, 10};
    for (int r : railwayRows) {
        line(OFFSET_X, OFFSET_Y + r * GRID_H, OFFSET_X + (GameLogic::COLS - 1) * GRID_W, OFFSET_Y + r * GRID_H);
    }
    // 纵向铁路
    line(OFFSET_X, OFFSET_Y + 1 * GRID_H, OFFSET_X, OFFSET_Y + 10 * GRID_H);
    line(OFFSET_X + 4 * GRID_W, OFFSET_Y + 1 * GRID_H, OFFSET_X + 4 * GRID_W, OFFSET_Y + 10 * GRID_H);

    // 5. 绘制行营周围的斜向连接线
    setlinestyle(PS_SOLID, 1);
    setlinecolor(BLACK);
    struct Connection { int r1, c1, r2, c2; };
    std::vector<Connection> connections = {
        // 上半部行营连接
        {1,0, 2,1}, {1,2, 2,1}, {3,0, 2,1}, {3,2, 2,1}, {2,0, 2,1}, {2,2, 2,1}, {1,1, 2,1}, {3,1, 2,1},
        {1,2, 2,3}, {1,4, 2,3}, {3,2, 2,3}, {3,4, 2,3}, {2,2, 2,3}, {2,4, 2,3}, {1,3, 2,3}, {3,3, 2,3},
        {2,1, 3,2}, {2,3, 3,2}, {4,1, 3,2}, {4,3, 3,2}, {3,1, 3,2}, {3,3, 3,2}, {2,2, 3,2}, {4,2, 3,2},
        {3,0, 4,1}, {3,2, 4,1}, {5,0, 4,1}, {5,2, 4,1}, {4,0, 4,1}, {4,2, 4,1}, {3,1, 4,1}, {5,1, 4,1},
        {3,2, 4,3}, {3,4, 4,3}, {5,2, 4,3}, {5,4, 4,3}, {4,2, 4,3}, {4,4, 4,3}, {3,3, 4,3}, {5,3, 4,3},
        // 下半部行营连接
        {6,0, 7,1}, {6,2, 7,1}, {8,0, 7,1}, {8,2, 7,1}, {7,0, 7,1}, {7,2, 7,1}, {6,1, 7,1}, {8,1, 7,1},
        {6,2, 7,3}, {6,4, 7,3}, {8,2, 7,3}, {8,4, 7,3}, {7,2, 7,3}, {7,4, 7,3}, {6,3, 7,3}, {8,3, 7,3},
        {7,1, 8,2}, {7,3, 8,2}, {9,1, 8,2}, {9,3, 8,2}, {8,1, 8,2}, {8,3, 8,2}, {7,2, 8,2}, {9,2, 8,2},
        {8,0, 9,1}, {8,2, 9,1}, {10,0, 9,1}, {10,2, 9,1}, {9,0, 9,1}, {9,2, 9,1}, {8,1, 9,1}, {10,1, 9,1},
        {8,2, 9,3}, {8,4, 9,3}, {10,2, 9,3}, {10,4, 9,3}, {9,2, 9,3}, {9,4, 9,3}, {8,3, 9,3}, {10,3, 9,3}
    };
    for (auto& c : connections) {
        line(OFFSET_X + c.c1 * GRID_W, OFFSET_Y + c.r1 * GRID_H, OFFSET_X + c.c2 * GRID_W, OFFSET_Y + c.r2 * GRID_H);
    }

    // 6. 绘制行营、大本营和普通站点
    for (int r = 0; r < GameLogic::ROWS; ++r) {
        for (int c = 0; c < GameLogic::COLS; ++c) {
            Pos p = {r, c};
            int x = OFFSET_X + c * GRID_W;
            int y = OFFSET_Y + r * GRID_H;
            if (game.isSafeZone(p)) {
                // 行营：圆形保护区
                setfillcolor(RGB(240, 230, 180));
                fillcircle(x, y, 27); 
            } else if (game.isHeadquarters(p)) {
                // 大本营：矩形区域
                setfillcolor(RGB(180, 140, 100));
                fillrectangle(x - 52, y - 27, x + 52, y + 27); 
            } else {
                // 普通公路站点：小圆点
                setfillcolor(BLACK);
                solidcircle(x, y, 5); 
            }
        }
    }

    // 7. 绘制所有棋子
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

// 绘制单个棋子
void Drawer::drawPiece(Pos p, Piece piece, bool isSelected) {
    int x = OFFSET_X + p.c * GRID_W;
    int y = OFFSET_Y + p.r * GRID_H;

    // 1. 如果未翻开，绘制背面
    if (!piece.isVisible) {
        setfillcolor(RGB(120, 120, 120)); // 灰色背面
        setlinecolor(BLACK);
        fillroundrect(x - 45, y - 25, x + 45, y + 25, 8, 8);
        return;
    }

    // 2. 确定棋子颜色
    if (piece.side == Side::SIDE_RED) {
        setfillcolor(RGB(255, 100, 100)); // 红方
    } else {
        setfillcolor(RGB(100, 100, 255)); // 蓝方
    }

    // 3. 选中状态高亮
    if (isSelected) {
        setlinecolor(YELLOW);
        setlinestyle(PS_SOLID, 3);
    } else {
        setlinecolor(BLACK);
        setlinestyle(PS_SOLID, 1);
    }

    // 4. 绘制棋子主体
    fillroundrect(x - 45, y - 25, x + 45, y + 25, 8, 8);
    setlinestyle(PS_SOLID, 1);

    // 5. 绘制棋子文字内容
    const wchar_t* text = L"";
    switch (piece.type) {
        case PieceType::PT_COMMANDER: text = L"司"; break;
        case PieceType::PT_CORPS:     text = L"军"; break;
        case PieceType::PT_DIVISION:  text = L"师"; break;
        case PieceType::PT_BRIGADE:   text = L"旅"; break;
        case PieceType::PT_REGIMENT:  text = L"团"; break;
        case PieceType::PT_BATTALION: text = L"营"; break;
        case PieceType::PT_COMPANY:   text = L"连"; break;
        case PieceType::PT_PLATOON:   text = L"排"; break;
        case PieceType::PT_ENGINEER:  text = L"工"; break;
        case PieceType::PT_BOMB:       text = L"炸"; break;
        case PieceType::PT_MINE:       text = L"雷"; break;
        case PieceType::PT_FLAG:       text = L"旗"; break;
        default: break;
    }

    settextcolor(WHITE);
    settextstyle(28, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);
    int tw = textwidth(text);
    int th = textheight(text);
    outtextxy(x - tw / 2, y - th / 2, text);
}

// 绘制游戏 UI 状态
void Drawer::drawUI(const GameLogic& game, int winner, int difficultyIndex) {
    // 绘制回合状态
    settextcolor(BLACK);
    settextstyle(30, 0, L"微软雅黑");
    
    std::wstring turnStr;
    if (winner == 0) {
        if (game.getCurrentTurn() == Side::SIDE_RED) {
            settextcolor(RGB(200, 0, 0));
            turnStr = L"当前回合：玩家 (红方)";
        } else {
            settextcolor(RGB(0, 0, 200));
            turnStr = L"当前回合：AI (蓝方)";
        }
    } else {
        turnStr = L"游戏结束";
    }
    
    outtextxy(20, 20, turnStr.c_str());

    // 绘制规则说明
    drawRules();
    drawDifficultySelector(difficultyIndex);

    // 绘制胜负信息
    if (winner != 0) {
        settextcolor(RED);
        settextstyle(60, 0, L"微软雅黑");
        const wchar_t* winStr = (winner == 1) ? L"红方胜利！" : L"蓝方胜利！";
        
        int tw = textwidth(winStr);
        int th = textheight(winStr);
        outtextxy((WIN_WIDTH - tw) / 2, (WIN_HEIGHT - th) / 2, winStr);
    }
}

void Drawer::drawDifficultySelector(int difficultyIndex) {
    const int panelLeft = 24;
    const int panelTop = WIN_HEIGHT - 170;
    const int panelRight = 250;
    const int panelBottom = WIN_HEIGHT - 24;
    const int optionLeft = panelLeft + 16;
    const int optionRight = panelRight - 16;
    const int optionWidth = optionRight - optionLeft;
    const int optionHeight = 40;
    const int firstOptionTop = panelTop + 52;
    const int secondOptionTop = firstOptionTop + optionHeight + 14;

    setlinecolor(RGB(120, 100, 70));
    setfillcolor(RGB(240, 226, 190));
    fillroundrect(panelLeft, panelTop, panelRight, panelBottom, 12, 12);

    setbkmode(TRANSPARENT);
    settextcolor(RGB(60, 45, 25));
    settextstyle(24, 0, L"微软雅黑", 0, 0, FW_BOLD, false, false, false);
    outtextxy(panelLeft + 16, panelTop + 14, L"难度选择");

    setfillcolor(difficultyIndex == 0 ? RGB(214, 122, 76) : RGB(227, 212, 178));
    setlinecolor(RGB(120, 100, 70));
    fillroundrect(optionLeft, firstOptionTop, optionRight, firstOptionTop + optionHeight, 10, 10);
    settextcolor(difficultyIndex == 0 ? WHITE : RGB(75, 55, 35));
    settextstyle(22, 0, L"微软雅黑");
    const wchar_t* currentText = L"弱鸡";
    int currentTw = textwidth(currentText);
    int currentTh = textheight(currentText);
    outtextxy(optionLeft + (optionWidth - currentTw) / 2, firstOptionTop + (optionHeight - currentTh) / 2, currentText);

    setfillcolor(difficultyIndex == 1 ? RGB(68, 110, 188) : RGB(210, 220, 240));
    setlinecolor(RGB(90, 110, 150));
    fillroundrect(optionLeft, secondOptionTop, optionRight, secondOptionTop + optionHeight, 10, 10);
    settextcolor(difficultyIndex == 1 ? WHITE : RGB(55, 75, 120));
    const wchar_t* reservedText = L"太逗";
    int reservedTw = textwidth(reservedText);
    int reservedTh = textheight(reservedText);
    outtextxy(optionLeft + (optionWidth - reservedTw) / 2, secondOptionTop + (optionHeight - reservedTh) / 2, reservedText);
}

// 绘制右侧规则说明
void Drawer::drawRules() {
    int startX = OFFSET_X + (GameLogic::COLS - 1) * GRID_W + 40;
    int startY = 100;
    
    settextcolor(RGB(50, 50, 50));
    settextstyle(28, 0, L"微软雅黑", 0, 0, FW_BOLD, false, false, false);
    outtextxy(startX, startY, L"游戏规则说明：");
    
    settextstyle(22, 0, L"微软雅黑");
    int lineHeight = 32;
    int curY = startY + 44;
    
    const wchar_t* rules[] = {
        L"1. 翻棋模式：初始棋子背面朝上",
        L"2. 双击翻牌，单击选择/移动",
        L"3. 行营规则：",
        L"   - 行营内不可被攻击",
        L"   - 地雷、炸弹不可进入行营",
        L"   - 行营周围支持斜向移动",
        L"4. 棋子限制：",
        L"   - 军旗、地雷不可移动",
        L"   - 炸弹可与任何棋子同归于尽",
        L"   - 地雷仅工兵可拆除",
        L"5. 铁路规则：",
        L"   - 铁路上可直线长距离移动",
        L"6. 胜负判定：",
        L"   - 吃掉军旗或对方无法移动",
        L"   - 需拆除所有地雷方可吃军旗"
    };
    
    for (const wchar_t* rule : rules) {
        outtextxy(startX, curY, rule);
        curY += lineHeight;
    }
}

int Drawer::hitTestDifficultyOption(int x, int y) const {
    const int panelTop = WIN_HEIGHT - 170;
    const int optionLeft = 40;
    const int optionRight = 234;
    const int optionHeight = 40;
    const int firstOptionTop = panelTop + 52;
    const int secondOptionTop = firstOptionTop + optionHeight + 14;

    if (x >= optionLeft && x <= optionRight && y >= firstOptionTop && y <= firstOptionTop + optionHeight) {
        return 0;
    }
    if (x >= optionLeft && x <= optionRight && y >= secondOptionTop && y <= secondOptionTop + optionHeight) {
        return 1;
    }
    return -1;
}

// 屏幕坐标转棋盘网格坐标
Pos Drawer::screenToBoard(int x, int y) {
    int c = (x - OFFSET_X + GRID_W / 2) / GRID_W;
    int r = (y - OFFSET_Y + GRID_H / 2) / GRID_H;
    return {r, c};
}

// 显示提示消息（暂未实现）
void Drawer::showMessage(const std::string& msg) {}
