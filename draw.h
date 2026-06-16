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
    void drawUI(const GameLogic& game, int winner, int difficultyIndex, int volume, bool musicEnabled,
        const std::wstring& announcementText, bool inspectArmed, int inspectCooldownTurns); // 绘制 UI 状态信息
    void drawRules();                                       // 绘制右侧规则说明
    void drawSystemAnnouncement(const std::wstring& announcementText); // 绘制系统公告栏
    void drawAudioControls(int volume, bool musicEnabled);   // 绘制左侧中部音频控制
    void drawInspectSkillPanel(bool inspectArmed, int inspectCooldownTurns, bool defaultMode); // 绘制验牌技能面板
    void drawDifficultySelector(int difficultyIndex);       // 绘制左下角难度选择
    int hitTestAudioControl(int x, int y) const;            // 点击命中音频控制
    int hitTestInspectSkillButton(int x, int y) const;      // 点击命中验牌技能按钮
    int audioVolumeFromSliderX(int x) const;                 // 根据滑条位置计算音量
    int hitTestDifficultyOption(int x, int y) const;        // 点击命中难度选项
    Pos screenToBoard(int x, int y);                        // 屏幕坐标转棋盘坐标
    void showMessage(const std::string& msg);               // 显示提示消息
};
