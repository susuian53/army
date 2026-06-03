#pragma once
#include "game.h"
#include "easyx_config.h"
#include <string>

// 图形绘制类
class Drawer {
public:
    // 界面尺寸配置
    static const int WIN_WIDTH = 1080; // 窗口宽度
    static const int WIN_HEIGHT = 810; // 窗口高度
    static const int GRID_W = 120;     // 格子宽度 (16:9 比例)
    static const int GRID_H = 68;      // 格子高度 (16:9 比例)
    static const int OFFSET_X = 300;   // 棋盘水平偏移
    static const int OFFSET_Y = 33;    // 棋盘垂直偏移

    void drawBoard(const GameLogic& game, Pos selectedPos); // 绘制整个棋盘
    void drawPiece(Pos p, Piece piece, bool isSelected);   // 绘制单个棋子
    void drawUI(const GameLogic& game, int winner);         // 绘制 UI 状态信息
    void drawRules();                                       // 绘制右侧规则说明
    Pos screenToBoard(int x, int y);                        // 屏幕坐标转棋盘坐标
    void showMessage(const std::string& msg);               // 显示提示消息
};
