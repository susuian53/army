#include "game.h"
#include "draw.h"
#include "music.h"
#include <windows.h>
#include <shellapi.h>
#include <chrono>
#include <array>

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

int main() {
    // 1. Initialize graphics window (初始化图形窗口)
    initgraph(Drawer::WIN_WIDTH, Drawer::WIN_HEIGHT);
    SetWindowText(GetHWnd(), L"GitHub \u4e0b\u8f7d\u5730\u5740: https://github.com/susuian53/army");

    GameLogic game;
    Drawer drawer;
    MusicPlayer music;

    Pos selectedPos = {-1, -1};
    int winner = 0;
    int difficultyIndex = 0;
    bool backgroundMusicEnabled = true;
    bool inspectArmed = false;
    int inspectCooldownTurns = 0;
    std::wstring announcementText = L"暂无系统公告";
    bool running = true;
    bool aiThinking = false;
    bool victoryMusicStarted = false;

    auto setAnnouncement = [&](const std::wstring& text) {
        announcementText = text;
    };

    auto pieceTypeText = [](PieceType type) -> std::wstring {
        switch (type) {
            case PieceType::PT_COMMANDER: return L"司令";
            case PieceType::PT_CORPS:     return L"军长";
            case PieceType::PT_DIVISION:   return L"师长";
            case PieceType::PT_BRIGADE:    return L"旅长";
            case PieceType::PT_REGIMENT:   return L"团长";
            case PieceType::PT_BATTALION:  return L"营长";
            case PieceType::PT_COMPANY:    return L"连长";
            case PieceType::PT_PLATOON:    return L"排长";
            case PieceType::PT_ENGINEER:   return L"工兵";
            case PieceType::PT_BOMB:       return L"炸弹";
            case PieceType::PT_MINE:       return L"地雷";
            case PieceType::PT_FLAG:       return L"军旗";
            default: return L"空位";
        }
    };

    auto sideText = [](Side side) -> std::wstring {
        if (side == Side::SIDE_RED) return L"红方";
        if (side == Side::SIDE_BLUE) return L"蓝方";
        return L"无阵营";
    };

    auto pieceText = [&](const Piece& piece) -> std::wstring {
        if (piece.type == PieceType::PT_EMPTY) return L"空位";
        return sideText(piece.side) + pieceTypeText(piece.type);
    };

    auto pieceKindText = [&](const Piece& piece) -> std::wstring {
        return pieceTypeText(piece.type);
    };

    auto captureBoard = [&]() {
        std::array<std::array<Piece, GameLogic::COLS>, GameLogic::ROWS> boardSnapshot{};
        for (int r = 0; r < GameLogic::ROWS; ++r) {
            for (int c = 0; c < GameLogic::COLS; ++c) {
                boardSnapshot[r][c] = game.getPiece({r, c});
            }
        }
        return boardSnapshot;
    };

    auto describeAIAction = [&](const auto& before, const auto& after) -> std::wstring {
        int changedCount = 0;
        int sourceCount = 0;
        int targetCount = 0;
        Pos sourcePos = {-1, -1};
        Pos targetPos = {-1, -1};
        Pos changedPos = {-1, -1};

        for (int r = 0; r < GameLogic::ROWS; ++r) {
            for (int c = 0; c < GameLogic::COLS; ++c) {
                if (before[r][c].type != after[r][c].type || before[r][c].side != after[r][c].side || before[r][c].isVisible != after[r][c].isVisible) {
                    ++changedCount;
                    changedPos = {r, c};
                }
                bool beforeEmpty = before[r][c].type == PieceType::PT_EMPTY;
                bool afterEmpty = after[r][c].type == PieceType::PT_EMPTY;
                if (!beforeEmpty && afterEmpty) {
                    sourcePos = {r, c};
                    ++sourceCount;
                } else if (beforeEmpty && !afterEmpty) {
                    targetPos = {r, c};
                    ++targetCount;
                }
            }
        }

        if (changedCount == 1 && changedPos.r >= 0) {
            Piece beforePiece = before[changedPos.r][changedPos.c];
            Piece afterPiece = after[changedPos.r][changedPos.c];
            if (beforePiece.type == afterPiece.type && beforePiece.side == afterPiece.side && !beforePiece.isVisible && afterPiece.isVisible) {
                return L"蓝方翻开了" + pieceText(afterPiece);
            }
        }

        if (sourceCount == 1 && targetCount == 1) {
            Piece movedPiece = after[targetPos.r][targetPos.c];
            return L"蓝方移动了" + pieceText(movedPiece);
        }

        if (sourceCount >= 1 && targetCount == 0) {
            return L"蓝方进行了战斗";
        }

        return L"蓝方已行动";
    };

    auto setPlayerAnnouncement = [&](const std::wstring& prefix, const Piece& actor, const Piece& target, int result) {
        if (result == 1) {
            setAnnouncement(prefix + L"移动了" + pieceKindText(actor));
        } else if (result == 2) {
            if (target.type == PieceType::PT_EMPTY) {
                setAnnouncement(prefix + L"行动了" + pieceKindText(actor));
            } else {
                setAnnouncement(prefix + L"用" + pieceKindText(actor) + L"吃掉了" + pieceText(target));
            }
        }
    };

    auto resetGame = [&](bool restartMusic) {
        game.setDifficultyMode(difficultyIndex == 0 ? DifficultyMode::CLASSIC : DifficultyMode::OPEN_STRONG);
        game.initBoard();
        selectedPos = {-1, -1};
        winner = 0;
        aiThinking = false;
        victoryMusicStarted = false;
        inspectArmed = false;
        inspectCooldownTurns = 0;
        if (restartMusic) {
            music.stopAll();
            if (backgroundMusicEnabled) {
                music.playBackgroundMusic();
            }
        }
    };

    // 设置音乐文件夹并启动背景音乐（使用相对路径解析）
    std::wstring musicFolder = resolveMusicFolder();
    music.setMusicFolder(musicFolder);
    if (backgroundMusicEnabled) {
        music.playBackgroundMusic();
    }

    // 2. High-precision timers for interaction (高精度计时器)
    auto lastClickTime = std::chrono::steady_clock::now();
    Pos lastClickPos = {-1, -1};
    
    auto aiTurnStartTime = std::chrono::steady_clock::now();

    ExMessage msg;
    const wchar_t* githubUrl = L"https://github.com/susuian53/army";

    while (running) {
        // --- 1. WIN CHECK (胜负判定) ---
        winner = game.checkWinner();

        // --- 1.1 VICTORY MUSIC HANDLING (胜利音乐处理) ---
        if (winner != 0 && backgroundMusicEnabled) {
            if (!victoryMusicStarted) {
                music.playVictoryMusic();
                victoryMusicStarted = true;
            } else if (!music.isVictoryMusicPlaying()) {
                // Music finished, reset game (音乐播放结束，重置游戏)
                resetGame(false);
                setAnnouncement(L"胜负结束，已重新开始");
                // 重新开启背景音乐
                if (backgroundMusicEnabled) {
                    music.playBackgroundMusic();
                }
            }
        }

        // --- 1.2 BACKGROUND MUSIC LOOP (背景音乐循环维持) ---
        // MCI 的 repeat 关键字在某些系统上不支持，手动维持循环
        if (backgroundMusicEnabled && winner == 0 && !victoryMusicStarted && !music.isBackgroundMusicPlaying()) {
            music.playBackgroundMusic();
        }

        // --- 2. ASYNCHRONOUS AI LOGIC (异步AI逻辑) ---
        if (winner == 0 && game.getCurrentTurn() == Side::SIDE_BLUE) {
            if (!aiThinking) {
                aiThinking = true;
                aiTurnStartTime = std::chrono::steady_clock::now();
            } else {
                auto now = std::chrono::steady_clock::now();
                // AI moves after 600ms delay (AI思考延迟)
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - aiTurnStartTime).count() > 600) {
                    auto before = captureBoard();
                    game.aiMove();
                    aiThinking = false;
                    if (inspectCooldownTurns > 0) {
                        --inspectCooldownTurns;
                    }
                    setAnnouncement(describeAIAction(before, captureBoard()));
                }
            }
        } else {
            aiThinking = false;
        }

        // --- 3. RENDERING (Double Buffering) (渲染逻辑) ---
        BeginBatchDraw();
        drawer.drawBoard(game, selectedPos);
        drawer.drawUI(game, winner, difficultyIndex, music.getVolume(), backgroundMusicEnabled,
            announcementText, inspectArmed, inspectCooldownTurns);
        EndBatchDraw();

        // --- 4. INPUT PROCESSING (消息处理) ---
        while (peekmessage(&msg, EM_MOUSE | EM_KEY)) {
            if (msg.message == WM_LBUTTONDOWN) {
                if (drawer.hitTestGithubButton(msg.x, msg.y) != -1) {
                    ShellExecuteW(NULL, L"open", githubUrl, NULL, NULL, SW_SHOWNORMAL);
                    setAnnouncement(L"已打开 GitHub 主页");
                    continue;
                }

                int audioHit = drawer.hitTestAudioControl(msg.x, msg.y);
                if (audioHit != -1) {
                    if (audioHit == 0) {
                        music.setVolume(music.getVolume() - 100);
                    } else if (audioHit == 1) {
                        music.setVolume(music.getVolume() + 100);
                    } else if (audioHit == 2) {
                        music.setVolume(drawer.audioVolumeFromSliderX(msg.x));
                    } else if (audioHit == 3) {
                        backgroundMusicEnabled = !backgroundMusicEnabled;
                        if (!backgroundMusicEnabled) {
                            music.stopAll();
                        } else {
                            if (winner == 0 && !victoryMusicStarted) {
                                music.playBackgroundMusic();
                            } else if (winner != 0) {
                                music.playVictoryMusic();
                                victoryMusicStarted = true;
                            }
                        }
                    }
                    continue;
                }

                int inspectHit = drawer.hitTestInspectSkillButton(msg.x, msg.y);
                if (inspectHit != -1) {
                    PlaySound(L"SystemExclamation", NULL, SND_ASYNC | SND_ALIAS);
                    music.playInspectSkill();
                    if (difficultyIndex != 0) {
                        setAnnouncement(L"明棋模式无需验牌");
                    } else if (inspectCooldownTurns > 0) {
                        setAnnouncement(L"验牌冷却中，请等待" + std::to_wstring(inspectCooldownTurns) + L"回合");
                    } else {
                        inspectArmed = true;
                        setAnnouncement(L"验牌已开启，请点击一张牌 | " + music.getLastInspectDebugInfo());
                    }
                    continue;
                }

                if (winner != 0) {
                    continue;
                }

                if (inspectArmed && difficultyIndex == 0) {
                    Pos inspectPos = drawer.screenToBoard(msg.x, msg.y);
                    if (game.isWithinBoard(inspectPos)) {
                        Piece inspectedPiece = game.getPiece(inspectPos);
                        if (inspectedPiece.type != PieceType::PT_EMPTY) {
                            setAnnouncement(L"验牌结果：" + pieceText(inspectedPiece));
                            inspectArmed = false;
                            inspectCooldownTurns = 3;
                        } else {
                            setAnnouncement(L"请点击一张有效棋子");
                        }
                        continue;
                    }
                }

                int difficultyHit = drawer.hitTestDifficultyOption(msg.x, msg.y);
                if (difficultyHit != -1) {
                    if (difficultyHit != difficultyIndex) {
                        difficultyIndex = difficultyHit;
                        resetGame(true);
                        setAnnouncement(difficultyIndex == 0 ? L"已切换到默认模式" : L"已切换到明棋模式");
                    }
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
                            setAnnouncement(sideText(Side::SIDE_RED) + L"翻开了" + pieceText(game.getPiece(clickedPos)));
                        }
                    }
                } else {
                    // Single Click: Select or Move (单击选择或移动)
                    if (selectedPos.r == -1) {
                        // Select piece (选择棋子)
                        if (p.isVisible && p.side == game.getCurrentTurn()) {
                            selectedPos = clickedPos;
                            music.playSelect();
                            setAnnouncement(L"已选中" + pieceText(p));
                        }
                    } else {
                        // Attempt Move (执行移动)
                        Piece movingPiece = game.getPiece(selectedPos);
                        Piece targetPiece = game.getPiece(clickedPos);
                        int res = game.movePiece(selectedPos, clickedPos);
                        if (res > 0) {
                            if (res == 1) music.playMove();
                            else music.playCapture();
                            setPlayerAnnouncement(sideText(Side::SIDE_RED), movingPiece, targetPiece, res);
                            selectedPos = {-1, -1};
                            if (inspectCooldownTurns > 0) {
                                --inspectCooldownTurns;
                            }
                        } else {
                            // Reselect or deselect (重选或取消)
                            if (p.isVisible && p.side == game.getCurrentTurn()) {
                                selectedPos = clickedPos;
                                music.playSelect();
                                setAnnouncement(L"已选中" + pieceText(p));
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
