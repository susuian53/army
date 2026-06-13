#include "game.h"
#include "draw.h"
#include "music.h"
#include <windows.h>
#include <chrono>
#include <fstream>
#include <sstream>

// 根据可执行文件路径解析音乐文件夹（支持从不同目录启动）
std::wstring resolveMusicFolder() {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring path(exePath);
    // 从 exe 路径向上查找，定位到项目根目录下的 电台节目 文件夹
    // exe 在 x64\Debug\Army.exe 或 Army\x64\Debug\Army.exe
    auto pos = path.rfind(L'\\');
    if (pos != std::wstring::npos) path = path.substr(0, pos);
    pos = path.rfind(L'\\');
    if (pos != std::wstring::npos) path = path.substr(0, pos);
    // 再往上一级，到达项目根目录
    pos = path.rfind(L'\\');
    if (pos != std::wstring::npos) path = path.substr(0, pos);
    path += L"\\电台节目";
    return path;
}

// #region debug-point A:report-helper
namespace {
struct DebugConfig {
    std::string url = "http://127.0.0.1:7777/event";
    std::string sessionId = "ai-turn-stuck";
};

DebugConfig loadDebugConfig() {
    DebugConfig cfg;
    std::ifstream in(".dbg/ai-turn-stuck.env");
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("DEBUG_SERVER_URL=", 0) == 0) cfg.url = line.substr(17);
        if (line.rfind("DEBUG_SESSION_ID=", 0) == 0) cfg.sessionId = line.substr(17);
    }
    return cfg;
}

void debugReport(const char* hypothesisId, const char* location, const std::string& msg, const std::string& dataJson) {
    static DebugConfig cfg = loadDebugConfig();
    std::string body =
        "{\"sessionId\":\"" + cfg.sessionId +
        "\",\"runId\":\"pre-fix\"" +
        ",\"hypothesisId\":\"" + std::string(hypothesisId) +
        "\",\"location\":\"" + std::string(location) +
        "\",\"msg\":\"[DEBUG] " + msg +
        "\",\"data\":" + dataJson + "}";
    std::string cmd =
        "powershell -NoProfile -Command \"try { Invoke-WebRequest -UseBasicParsing -Uri '" + cfg.url +
        "' -Method Post -ContentType 'application/json' -Body '" + body +
        "' | Out-Null } catch {}\" >nul 2>nul";
    std::system(cmd.c_str());
}
} // namespace
// #endregion

int main() {
    // 1. Initialize graphics window (初始化图形窗口)
    initgraph(Drawer::WIN_WIDTH, Drawer::WIN_HEIGHT);
    SetWindowText(GetHWnd(), L"\u519b\u65d7\u6a21\u62df\u6e38\u620f - \u6781\u901f\u54cd\u5e94\u7248");

    GameLogic game;
    Drawer drawer;
    MusicPlayer music;

    Pos selectedPos = {-1, -1};
    int winner = 0;
    int difficultyIndex = 0;
    bool running = true;
    bool victoryMusicStarted = false;
    auto aiTurnStartTime = std::chrono::steady_clock::now();
    bool aiThinking = false;

    auto resetGame = [&](bool restartMusic) {
        game.setDifficultyMode(difficultyIndex == 0 ? DifficultyMode::CLASSIC : DifficultyMode::OPEN_STRONG);
        game.initBoard();
        selectedPos = {-1, -1};
        winner = 0;
        aiThinking = false;
        victoryMusicStarted = false;
        if (restartMusic) {
            music.stopAll();
            music.playBackgroundMusic();
        }
    };

    // 设置音乐文件夹并启动背景音乐（使用相对路径解析）
    std::wstring musicFolder = resolveMusicFolder();
    music.setMusicFolder(musicFolder);
    music.playBackgroundMusic();

    // 2. High-precision timers for interaction (高精度计时器)
    auto lastClickTime = std::chrono::steady_clock::now();
    Pos lastClickPos = {-1, -1};

    ExMessage msg;

    while (running) {
        // --- 1. WIN CHECK (胜负判定) ---
        winner = game.checkWinner();

        // --- 1.1 VICTORY MUSIC HANDLING (胜利音乐处理) ---
        if (winner != 0) {
            if (!victoryMusicStarted) {
                music.playVictoryMusic(musicFolder);
                victoryMusicStarted = true;
            } else if (!music.isVictoryMusicPlaying()) {
                // Music finished, reset game (音乐播放结束，重置游戏)
                resetGame(false);
                // 重新开启背景音乐
                music.playBackgroundMusic();
            }
        }

        // --- 1.2 BACKGROUND MUSIC LOOP (背景音乐循环维持) ---
        // MCI 的 repeat 关键字在某些系统上不支持，手动维持循环
        if (winner == 0 && !victoryMusicStarted && !music.isBackgroundMusicPlaying()) {
            music.playBackgroundMusic();
        }

        // --- 2. ASYNCHRONOUS AI LOGIC (异步AI逻辑) ---
        if (winner == 0 && game.getCurrentTurn() == Side::SIDE_BLUE) {
            if (!aiThinking) {
                aiThinking = true;
                aiTurnStartTime = std::chrono::steady_clock::now();
                // #region debug-point D:ai-turn-start
                debugReport("D", "main.cpp:128", "ai turn entered", "{\"difficultyIndex\":" + std::to_string(difficultyIndex) + "}");
                // #endregion
            } else {
                auto now = std::chrono::steady_clock::now();
                // AI moves after 600ms delay (AI思考延迟)
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - aiTurnStartTime).count() > 600) {
                    // #region debug-point A:before-ai-move
                    debugReport("A", "main.cpp:134", "calling game.aiMove", "{\"waitMs\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now - aiTurnStartTime).count()) + "}");
                    // #endregion
                    game.aiMove();
                    // #region debug-point D:after-ai-move
                    debugReport("D", "main.cpp:137", "game.aiMove returned", "{\"elapsedMs\":" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - aiTurnStartTime).count()) + ",\"nextTurn\":" + std::to_string(static_cast<int>(game.getCurrentTurn())) + "}");
                    // #endregion
                    aiThinking = false;
                }
            }
        } else {
            aiThinking = false;
        }

        // --- 3. RENDERING (Double Buffering) (渲染逻辑) ---
        BeginBatchDraw();
        drawer.drawBoard(game, selectedPos);
        drawer.drawUI(game, winner, difficultyIndex);
        EndBatchDraw();

        // --- 4. INPUT PROCESSING (消息处理) ---
        while (peekmessage(&msg, EM_MOUSE | EM_KEY)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int difficultyHit = drawer.hitTestDifficultyOption(msg.x, msg.y);
                if (difficultyHit != -1) {
                    if (difficultyHit != difficultyIndex) {
                        difficultyIndex = difficultyHit;
                        resetGame(true);
                    }
                    continue;
                }

                if (winner != 0 || game.getCurrentTurn() != Side::SIDE_RED) {
                    continue;
                }

                Pos clickedPos = drawer.screenToBoard(msg.x, msg.y);
                Piece p = game.getPiece(clickedPos);
                
                auto now = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastClickTime).count();

                // Double Click Detection (双击检测)
                if (duration < 300 && clickedPos == lastClickPos) {
                    // Double Click: Flip piece (双击翻牌)
                    if (p.type != PieceType::PT_EMPTY && !p.isVisible) {
                        if (game.flipPiece(clickedPos)) {
                            music.playFlip();
                            selectedPos = {-1, -1};
                        }
                    }
                } else {
                    // Single Click: Select or Move (单击选择或移动)
                    if (selectedPos.r == -1) {
                        // Select piece (选择棋子)
                        if (p.isVisible && p.side == game.getCurrentTurn()) {
                            selectedPos = clickedPos;
                            music.playSelect();
                        }
                    } else {
                        // Attempt Move (执行移动)
                        int res = game.movePiece(selectedPos, clickedPos);
                        if (res > 0) {
                            if (res == 1) music.playMove();
                            else music.playCapture();
                            selectedPos = {-1, -1};
                        } else {
                            // Reselect or deselect (重选或取消)
                            if (p.isVisible && p.side == game.getCurrentTurn()) {
                                selectedPos = clickedPos;
                                music.playSelect();
                            } else {
                                selectedPos = {-1, -1};
                            }
                        }
                    }
                }
                lastClickTime = now;
                lastClickPos = clickedPos;
            } else if (msg.message == WM_KEYDOWN) {
                if (msg.vkcode == 'R' || msg.vkcode == 'r') {
                    // Reset game (重置游戏)
                    resetGame(true);
                } else if (msg.vkcode == VK_ESCAPE) {
                    // Quit game (退出游戏)
                    running = false;
                }
            }
        }

        // --- 5. LOOP DELAY (循环延迟) ---
        Sleep(1); 
    }

    closegraph();
    return 0;
}
