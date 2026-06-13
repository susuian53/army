#pragma once
#include <chrono>
#include <vector>

// 阵营枚举
enum class Side { SIDE_RED, SIDE_BLUE, SIDE_NONE };

// 难度模式
enum class DifficultyMode {
    CLASSIC = 0,
    OPEN_STRONG = 1
};

// 棋子类型枚举
enum class PieceType { 
    PT_COMMANDER, // 司令
    PT_CORPS,     // 军长
    PT_DIVISION,  // 师长
    PT_BRIGADE,   // 旅长
    PT_REGIMENT,  // 团长
    PT_BATTALION, // 营长
    PT_COMPANY,   // 连长
    PT_PLATOON,   // 排长
    PT_ENGINEER,  // 工兵
    PT_BOMB,      // 炸弹
    PT_MINE,      // 地雷
    PT_FLAG,      // 军旗
    PT_EMPTY      // 空位
};

// 棋子结构体
struct Piece {
    PieceType type; // 类型
    Side side;      // 所属阵营
    bool isVisible; // 是否翻开（可见）
    Piece(PieceType t = PieceType::PT_EMPTY, Side s = Side::SIDE_NONE) : type(t), side(s), isVisible(false) {}
};

// 坐标结构体
struct Pos {
    int r, c; // 行、列
    bool operator==(const Pos& other) const { return r == other.r && c == other.c; }
    bool operator!=(const Pos& other) const { return !(*this == other); }
};

// 游戏逻辑类
class GameLogic {
public:
    static const int ROWS = 12; // 棋盘行数
    static const int COLS = 5;  // 棋盘列数
    
    GameLogic();
    void initBoard(); // 初始化棋盘
    void setDifficultyMode(DifficultyMode mode) { difficultyMode = mode; }
    DifficultyMode getDifficultyMode() const { return difficultyMode; }
    
    Piece getPiece(Pos p) const; // 获取指定位置的棋子
    Side getCurrentTurn() const { return currentTurn; } // 获取当前回合
    void switchTurn(); // 切换回合
    
    bool flipPiece(Pos p); // 翻开棋子
    int movePiece(Pos from, Pos to); // 移动棋子：返回0失败，1移动到空位，2发生战斗
    bool isValidMove(Pos from, Pos to) const; // 判断移动是否合法
    int checkWinner(); // 检查胜负：0继续，1红胜，2蓝胜
    void aiMove(); // AI 行动逻辑

    bool isSafeZone(Pos p) const;   // 是否为行营（保护区）
    bool isHeadquarters(Pos p) const; // 是否为大本营
    bool isRailway(Pos p) const;    // 是否为铁路
    bool isWithinBoard(Pos p) const; // 是否在棋盘范围内
    bool areConnected(Pos from, Pos to) const; // 两个位置是否有线条连接

private:
    struct Action {
        bool isFlip;
        Pos from;
        Pos to;
    };

    Piece board[ROWS][COLS]; // 棋盘数组
    Side currentTurn;         // 当前回合阵营
    DifficultyMode difficultyMode; // 当前难度模式
    
    int getRank(PieceType t) const; // 获取棋子等级
    bool canEat(Piece attacker, Piece target) const; // 判断是否可以吃子
    void revealAllPieces();
    std::vector<Action> generateActions(bool allowFlip) const;
    void applyAction(const Action& action);
    int evaluateBoard(Side side) const;
    int evaluateMaterial(Side side) const;
    int evaluateFlagSafety(Side side) const;
    int countMobility(Side side) const;
    int scoreActionForOrdering(const Action& action) const;
    int alphaBeta(int depth, int alpha, int beta, Side maximizingSide,
        std::chrono::steady_clock::time_point deadline, bool& timedOut) const;
    void aiMoveClassic();
    void aiMoveOpenStrong();
};
