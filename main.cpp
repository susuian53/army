#include "game.h"
#include "draw.h"
#include "music.h"
#include <windows.h>
#include <chrono>

int main() {
    // 1. Initialize graphics window (初始化图形窗口)
    initgraph(Drawer::WIN_WIDTH, Drawer::WIN_HEIGHT);
    SetWindowText(GetHWnd(), L"\u519b\u65d7\u6a21\u62df\u6e38\u620f - \u6781\u901f\u54cd\u5e94\u7248");

    GameLogic game;
    Drawer drawer;
    MusicPlayer music;

    Pos selectedPos = {-1, -1};
    int winner = 0;
    bool running = true;

    // 2. High-precision timers for interaction (高精度计时器)
    auto lastClickTime = std::chrono::steady_clock::now();
    Pos lastClickPos = {-1, -1};
    
    auto aiTurnStartTime = std::chrono::steady_clock::now();
    bool aiThinking = false;

    ExMessage msg;

    while (running) {
        // --- 1. WIN CHECK (胜负判定) ---
        winner = game.checkWinner();

        // --- 2. ASYNCHRONOUS AI LOGIC (异步AI逻辑) ---
        if (winner == 0 && game.getCurrentTurn() == Side::SIDE_BLUE) {
            if (!aiThinking) {
                aiThinking = true;
                aiTurnStartTime = std::chrono::steady_clock::now();
            } else {
                auto now = std::chrono::steady_clock::now();
                // AI moves after 600ms delay (AI思考延迟)
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - aiTurnStartTime).count() > 600) {
                    game.aiMove();
                    aiThinking = false;
                }
            }
        } else {
            aiThinking = false;
        }

        // --- 3. RENDERING (Double Buffering) (渲染逻辑) ---
        BeginBatchDraw();
        drawer.drawBoard(game, selectedPos);
        drawer.drawUI(game, winner);
        EndBatchDraw();

        // --- 4. INPUT PROCESSING (消息处理) ---
        while (peekmessage(&msg, EM_MOUSE | EM_KEY)) {
            if (msg.message == WM_LBUTTONDOWN && winner == 0) {
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
                    game.initBoard();
                    selectedPos = {-1, -1};
                    winner = 0;
                    aiThinking = false;
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
