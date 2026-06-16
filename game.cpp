#include "game.h"
#include <algorithm>
#include <chrono>
#include <limits>
#include <ctime>
#include <random>

namespace {
int pieceBaseValue(PieceType t) {
    switch (t) {
        case PieceType::PT_FLAG:       return 20000;
        case PieceType::PT_COMMANDER:  return 3200;
        case PieceType::PT_CORPS:      return 2500;
        case PieceType::PT_DIVISION:   return 1900;
        case PieceType::PT_BRIGADE:    return 1500;
        case PieceType::PT_REGIMENT:   return 1100;
        case PieceType::PT_BATTALION:  return 850;
        case PieceType::PT_COMPANY:    return 650;
        case PieceType::PT_PLATOON:    return 480;
        case PieceType::PT_ENGINEER:   return 950;
        case PieceType::PT_BOMB:       return 1600;
        case PieceType::PT_MINE:       return 900;
        default: return 0;
    }
}

Side oppositeSide(Side side) {
    if (side == Side::SIDE_RED) return Side::SIDE_BLUE;
    if (side == Side::SIDE_BLUE) return Side::SIDE_RED;
    return Side::SIDE_NONE;
}
} // namespace

GameLogic::GameLogic() {
    difficultyMode = DifficultyMode::CLASSIC;
    initBoard();
}

// Initialize the board
void GameLogic::initBoard() {
    currentTurn = Side::SIDE_RED;

    // 1. Clear board
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            board[r][c] = Piece(PieceType::PT_EMPTY, Side::SIDE_NONE);
        }
    }

    std::random_device rd;
    std::mt19937 g(rd());

    // 2. Create a pool of all 50 pieces (25 Red, 25 Blue)
    std::vector<Piece> allPieces;
    std::vector<PieceType> types = {
        PieceType::PT_COMMANDER, PieceType::PT_CORPS,
        PieceType::PT_DIVISION, PieceType::PT_DIVISION,
        PieceType::PT_BRIGADE, PieceType::PT_BRIGADE,
        PieceType::PT_REGIMENT, PieceType::PT_REGIMENT,
        PieceType::PT_BATTALION, PieceType::PT_BATTALION,
        PieceType::PT_COMPANY, PieceType::PT_COMPANY, PieceType::PT_COMPANY,
        PieceType::PT_PLATOON, PieceType::PT_PLATOON, PieceType::PT_PLATOON,
        PieceType::PT_ENGINEER, PieceType::PT_ENGINEER, PieceType::PT_ENGINEER,
        PieceType::PT_MINE, PieceType::PT_MINE, PieceType::PT_MINE
    };

    // Add Red and Blue versions (excluding Flag and Bomb for special placement)
    for (auto t : types) {
        allPieces.push_back(Piece(t, Side::SIDE_RED));
        allPieces.push_back(Piece(t, Side::SIDE_BLUE));
    }

    // 3. Special pieces
    Piece redFlag(PieceType::PT_FLAG, Side::SIDE_RED);
    Piece blueFlag(PieceType::PT_FLAG, Side::SIDE_BLUE);
    Piece redBomb(PieceType::PT_BOMB, Side::SIDE_RED);
    Piece blueBomb(PieceType::PT_BOMB, Side::SIDE_BLUE);

    // 4. Identify all valid spots (non-safe-zones)
    std::vector<Pos> hqSpots = {{0, 1}, {0, 3}, {11, 1}, {11, 3}}; // HQ positions
    std::vector<Pos> otherSpots;
    std::vector<Pos> bombAllowedSpots;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            Pos p = {r, c};
            if (isSafeZone(p)) continue; // Safe zones cannot have pieces initially
            
            bool isHQ = false;
            for(auto h : hqSpots) if(h == p) isHQ = true;
            
            if (!isHQ) {
                otherSpots.push_back(p);
                // Bombs cannot be in the first row (0 or 11)
                if (r != 0 && r != 11) {
                    bombAllowedSpots.push_back(p);
                }
            }
        }
    }

    // 5. Place Flags in random HQs
    std::shuffle(hqSpots.begin(), hqSpots.end(), g);
    board[hqSpots[0].r][hqSpots[0].c] = redFlag;
    board[hqSpots[1].r][hqSpots[1].c] = blueFlag;
    // Remaining 2 HQs become part of otherSpots
    otherSpots.push_back(hqSpots[2]);
    otherSpots.push_back(hqSpots[3]);

    // 6. Place Bombs (4 total: 2 Red, 2 Blue)
    std::shuffle(bombAllowedSpots.begin(), bombAllowedSpots.end(), g);
    std::vector<Piece> bombs = {redBomb, redBomb, blueBomb, blueBomb};
    for (int i = 0; i < 4; i++) {
        Pos p = bombAllowedSpots[i];
        board[p.r][p.c] = bombs[i];
        // Remove this spot from otherSpots
        otherSpots.erase(std::remove(otherSpots.begin(), otherSpots.end(), p), otherSpots.end());
    }

    // 7. Place remaining 44 pieces
    std::shuffle(allPieces.begin(), allPieces.end(), g);
    std::shuffle(otherSpots.begin(), otherSpots.end(), g);
    for (size_t i = 0; i < allPieces.size(); i++) {
        Pos p = otherSpots[i];
        board[p.r][p.c] = allPieces[i];
    }

    // Ensure all are hidden initially
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (board[r][c].type != PieceType::PT_EMPTY) board[r][c].isVisible = false;
        }
    }

    if (difficultyMode == DifficultyMode::OPEN_STRONG) {
        revealAllPieces();
    }
}

// Flip a piece
bool GameLogic::flipPiece(Pos p) {
    if (!isWithinBoard(p)) return false;
    if (board[p.r][p.c].type == PieceType::PT_EMPTY || board[p.r][p.c].isVisible) return false;

    board[p.r][p.c].isVisible = true;
    switchTurn(); // Switch turn after flipping
    return true;
}

Piece GameLogic::getPiece(Pos p) const {
    if (!isWithinBoard(p)) return Piece();
    return board[p.r][p.c];
}

void GameLogic::revealAllPieces() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (board[r][c].type != PieceType::PT_EMPTY) {
                board[r][c].isVisible = true;
            }
        }
    }
}

void GameLogic::switchTurn() {
    currentTurn = (currentTurn == Side::SIDE_RED) ? Side::SIDE_BLUE : Side::SIDE_RED;
}

bool GameLogic::isWithinBoard(Pos p) const {
    return p.r >= 0 && p.r < ROWS && p.c >= 0 && p.c < COLS;
}

// Check if a position is a Safe Zone
bool GameLogic::isSafeZone(Pos p) const {
    static const std::vector<Pos> sz = {
        {2,1}, {2,3}, {3,2}, {4,1}, {4,3},
        {7,1}, {7,3}, {8,2}, {9,1}, {9,3}
    };
    for (auto& s : sz) if (s == p) return true;
    return false;
}

// Check if a position is a Headquarters
bool GameLogic::isHeadquarters(Pos p) const {
    return (p.r == 0 && (p.c == 1 || p.c == 3)) || (p.r == 11 && (p.c == 1 || p.c == 3));
}

// Check if a position is a Railway
bool GameLogic::isRailway(Pos p) const {
    if (p.r == 1 || p.r == 5 || p.r == 6 || p.r == 10) return true;
    if ((p.c == 0 || p.c == 4) && p.r >= 1 && p.r <= 10) return true;
    return false;
}

// Get the rank of a piece type for comparison
int GameLogic::getRank(PieceType t) const {
    switch (t) {
        case PieceType::PT_COMMANDER: return 9;
        case PieceType::PT_CORPS:     return 8;
        case PieceType::PT_DIVISION:  return 7;
        case PieceType::PT_BRIGADE:   return 6;
        case PieceType::PT_REGIMENT:  return 5;
        case PieceType::PT_BATTALION: return 4;
        case PieceType::PT_COMPANY:   return 3;
        case PieceType::PT_PLATOON:   return 2;
        case PieceType::PT_ENGINEER:  return 1;
        default: return 0;
    }
}

// Check if an attacker can capture a target
bool GameLogic::canEat(Piece attacker, Piece target) const {
    if (target.type == PieceType::PT_EMPTY) return true;
    if (attacker.type == PieceType::PT_BOMB || target.type == PieceType::PT_BOMB) return true;
    if (target.type == PieceType::PT_MINE) {
        return attacker.type == PieceType::PT_ENGINEER;
    }
    if (target.type == PieceType::PT_FLAG) return true;

    int r1 = getRank(attacker.type);
    int r2 = getRank(target.type);
    return r1 >= r2;
}

// Check if two positions are directly connected on the map
bool GameLogic::areConnected(Pos from, Pos to) const {
    if (!isWithinBoard(from) || !isWithinBoard(to)) return false;
    int r1 = from.r, c1 = from.c, r2 = to.r, c2 = to.c;
    int dr = abs(r1 - r2), dc = abs(c1 - c2);

    // 1. Standard horizontal/vertical moves
    if ((dr == 1 && dc == 0) || (dr == 0 && dc == 1)) {
        // Special case: River crossing (between row 5 and 6)
        if ((r1 == 5 && r2 == 6) || (r1 == 6 && r2 == 5)) {
            // Only columns 0, 2, 4 have paths across the river
            return (c1 == c2) && (c1 == 0 || c1 == 2 || c1 == 4);
        }
        return true;
    }

    // 2. Safe Zone diagonal moves (8-way connectivity)
    if (isSafeZone(from) || isSafeZone(to)) {
        if (dr == 1 && dc == 1) return true;
    }

    return false;
}

// Check if a move from 'from' to 'to' is valid
bool GameLogic::isValidMove(Pos from, Pos to) const {
    if (!isWithinBoard(from) || !isWithinBoard(to)) return false;
    Piece f = board[from.r][from.c];
    Piece t = board[to.r][to.c];

    // Must be a visible piece of the current turn
    if (!f.isVisible || f.side != currentTurn) return false;
    
    // Flags and Mines cannot move
    if (f.type == PieceType::PT_MINE || f.type == PieceType::PT_FLAG) return false;
    
    // Cannot capture own pieces
    if (t.isVisible && t.side == currentTurn) return false;
    
    // Safe Zone restrictions
    if (isSafeZone(to)) {
        // Safe Zone is a non-attack zone: cannot enter if occupied
        if (t.type != PieceType::PT_EMPTY) return false;
        // Mines and Bombs cannot enter safe zones
        if (f.type == PieceType::PT_BOMB || f.type == PieceType::PT_MINE) return false;
    }

    // Rule: Must remove all enemy mines before capturing enemy flag
    if (t.isVisible && t.type == PieceType::PT_FLAG) {
        Side enemySide = t.side;
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (board[r][c].side == enemySide && board[r][c].type == PieceType::PT_MINE) {
                    return false; // Enemy still has mines
                }
            }
        }
    }

    // 1. Check direct connection (Standard or Safe Zone diagonal)
    if (areConnected(from, to)) return true;

    // 2. Railway move (long distance straight lines)
    if (isRailway(from) && isRailway(to)) {
        if (from.r == to.r) {
            int minC = std::min(from.c, to.c), maxC = std::max(from.c, to.c);
            for (int c = minC + 1; c < maxC; c++) if (board[from.r][c].type != PieceType::PT_EMPTY) return false;
            return true;
        }
        if (from.c == to.c && (from.c == 0 || from.c == 4)) {
            int minR = std::min(from.r, to.r), maxR = std::max(from.r, to.r);
            for (int r = minR + 1; r < maxR; r++) if (board[r][from.c].type != PieceType::PT_EMPTY) return false;
            return true;
        }
    }

    return false;
}

// Move a piece and handle captures/clashes
int GameLogic::movePiece(Pos from, Pos to) {
    if (!isValidMove(from, to)) return 0;

    Piece& f = board[from.r][from.c];
    Piece& t = board[to.r][to.c];

    // Cannot move into a hidden enemy piece directly
    if (t.type != PieceType::PT_EMPTY && !t.isVisible) return 0;

    int moveResult = 1; // Default: moved to empty

    if (t.type == PieceType::PT_EMPTY) {
        t = f;
        f = Piece();
    } else {
        moveResult = 2; // Capture or clash
        bool f_alive = true, t_alive = true;
        if (f.type == PieceType::PT_BOMB || t.type == PieceType::PT_BOMB) {
            f_alive = false; t_alive = false;
        } else if (t.type == PieceType::PT_MINE) {
            if (f.type == PieceType::PT_ENGINEER) t_alive = false;
            else f_alive = false;
        } else if (t.type == PieceType::PT_FLAG) {
            t_alive = false;
        } else {
            int r1 = getRank(f.type), r2 = getRank(t.type);
            if (r1 > r2) t_alive = false;
            else if (r1 < r2) f_alive = false;
            else { f_alive = false; t_alive = false; }
        }

        if (f_alive && !t_alive) { t = f; f = Piece(); }
        else if (!f_alive && t_alive) { f = Piece(); }
        else { f = Piece(); t = Piece(); }
    }

    switchTurn();
    return moveResult;
}

// Check for winner
int GameLogic::checkWinner() {
    bool redFlag = false, blueFlag = false;
    int redMovable = 0, blueMovable = 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            Piece p = board[r][c];
            if (p.type == PieceType::PT_EMPTY) continue;

            if (p.side == Side::SIDE_RED) {
                if (p.type == PieceType::PT_FLAG) redFlag = true;
                else if (p.type != PieceType::PT_MINE) redMovable++;
            } else if (p.side == Side::SIDE_BLUE) {
                if (p.type == PieceType::PT_FLAG) blueFlag = true;
                else if (p.type != PieceType::PT_MINE) blueMovable++;
            }
        }
    }

    if (!blueFlag) return 1; // Red wins
    if (!redFlag) return 2;  // Blue wins

    if (blueMovable == 0) return 1;
    if (redMovable == 0) return 2;

    return 0; // Ongoing
}

std::vector<GameLogic::Action> GameLogic::generateActions(bool allowFlip) const {
    std::vector<Action> actions;

    if (allowFlip) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                if (board[r][c].type != PieceType::PT_EMPTY && !board[r][c].isVisible) {
                    actions.push_back({true, {r, c}, {r, c}});
                }
            }
        }
    }

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            Pos from = {r, c};
            if (board[r][c].isVisible && board[r][c].side == currentTurn) {
                for (int tr = 0; tr < ROWS; tr++) {
                    for (int tc = 0; tc < COLS; tc++) {
                        Pos to = {tr, tc};
                        if (isValidMove(from, to)) {
                            actions.push_back({false, from, to});
                        }
                    }
                }
            }
        }
    }

    return actions;
}

void GameLogic::applyAction(const Action& action) {
    if (action.isFlip) {
        flipPiece(action.from);
    } else {
        movePiece(action.from, action.to);
    }
}

int GameLogic::evaluateMaterial(Side side) const {
    const Side enemy = oppositeSide(side);
    bool enemyHasMines = false;
    bool ownHasMines = false;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (board[r][c].type == PieceType::PT_MINE) {
                if (board[r][c].side == side) ownHasMines = true;
                if (board[r][c].side == enemy) enemyHasMines = true;
            }
        }
    }

    int score = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            Pos p = {r, c};
            const Piece& piece = board[r][c];
            if (piece.type == PieceType::PT_EMPTY) continue;

            int value = pieceBaseValue(piece.type);
            if (piece.type == PieceType::PT_ENGINEER) {
                if (piece.side == side && enemyHasMines) value += 300;
                if (piece.side == enemy && ownHasMines) value += 300;
            }
            if (piece.type == PieceType::PT_MINE && isHeadquarters(p)) value += 250;
            if (piece.type != PieceType::PT_FLAG && piece.type != PieceType::PT_MINE) {
                if (isRailway(p)) value += 40;
                if (isSafeZone(p)) value += 25;
            }

            score += (piece.side == side) ? value : -value;
        }
    }
    return score;
}

int GameLogic::evaluateFlagSafety(Side side) const {
    const Side enemy = oppositeSide(side);
    Pos ownFlag = {-1, -1};
    Pos enemyFlag = {-1, -1};

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (board[r][c].type == PieceType::PT_FLAG) {
                if (board[r][c].side == side) ownFlag = {r, c};
                if (board[r][c].side == enemy) enemyFlag = {r, c};
            }
        }
    }

    auto scoreSingleFlag = [&](Pos flagPos, Side owner) {
        if (flagPos.r == -1) return -30000;

        int value = isHeadquarters(flagPos) ? 900 : 0;
        for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
                if (dr == 0 && dc == 0) continue;
                Pos np = {flagPos.r + dr, flagPos.c + dc};
                if (!isWithinBoard(np)) continue;
                const Piece& guard = board[np.r][np.c];
                if (guard.type == PieceType::PT_EMPTY) continue;
                if (guard.side == owner) {
                    if (guard.type == PieceType::PT_MINE) value += 450;
                    else if (guard.type == PieceType::PT_BOMB) value += 260;
                    else value += 90;
                } else if (guard.isVisible) {
                    value -= 180;
                }
            }
        }
        return value;
    };

    return scoreSingleFlag(ownFlag, side) - scoreSingleFlag(enemyFlag, enemy);
}

int GameLogic::countMobility(Side side) const {
    GameLogic temp = *this;
    temp.currentTurn = side;
    return static_cast<int>(temp.generateActions(false).size());
}

int GameLogic::evaluateBoard(Side side) const {
    GameLogic temp = *this;
    int winner = temp.checkWinner();
    if (winner != 0) {
        Side winnerSide = (winner == 1) ? Side::SIDE_RED : Side::SIDE_BLUE;
        return (winnerSide == side) ? 100000000 : -100000000;
    }

    Side enemy = oppositeSide(side);
    int score = 0;
    score += evaluateMaterial(side);
    score += evaluateFlagSafety(side);
    score += 12 * (countMobility(side) - countMobility(enemy));

    GameLogic ownView = *this;
    ownView.currentTurn = side;
    for (const Action& action : ownView.generateActions(false)) {
        const Piece& target = board[action.to.r][action.to.c];
        if (target.type != PieceType::PT_EMPTY && target.side == enemy) {
            score += pieceBaseValue(target.type) / 8;
        }
    }

    GameLogic enemyView = *this;
    enemyView.currentTurn = enemy;
    for (const Action& action : enemyView.generateActions(false)) {
        const Piece& target = board[action.to.r][action.to.c];
        if (target.type != PieceType::PT_EMPTY && target.side == side) {
            score -= pieceBaseValue(target.type) / 8;
        }
    }

    return score;
}

int GameLogic::scoreActionForOrdering(const Action& action) const {
    if (action.isFlip) return 50;

    const Piece& mover = board[action.from.r][action.from.c];
    const Piece& target = board[action.to.r][action.to.c];
    int score = 0;

    if (target.type != PieceType::PT_EMPTY) {
        score += 50000 + pieceBaseValue(target.type) - pieceBaseValue(mover.type) / 10;
    }

    if (isRailway(action.to)) score += 120;
    if (isSafeZone(action.to)) score += 40;

    if (currentTurn == Side::SIDE_BLUE) score += (action.to.r - action.from.r) * 10;
    if (currentTurn == Side::SIDE_RED) score += (action.from.r - action.to.r) * 10;

    return score;
}

int GameLogic::alphaBeta(int depth, int alpha, int beta, Side maximizingSide,
    std::chrono::steady_clock::time_point deadline, bool& timedOut) const {
    if (timedOut) return 0;
    if (std::chrono::steady_clock::now() >= deadline) {
        timedOut = true;
        return 0;
    }

    GameLogic terminal = *this;
    if (depth == 0 || terminal.checkWinner() != 0) {
        return evaluateBoard(maximizingSide);
    }

    std::vector<Action> actions = generateActions(false);
    if (actions.empty()) {
        return evaluateBoard(maximizingSide);
    }

    std::sort(actions.begin(), actions.end(), [&](const Action& a, const Action& b) {
        return scoreActionForOrdering(a) > scoreActionForOrdering(b);
    });

    const bool maximizing = (currentTurn == maximizingSide);
    if (maximizing) {
        int bestScore = std::numeric_limits<int>::min();
        for (const Action& action : actions) {
            GameLogic next = *this;
            next.applyAction(action);
            int score = next.alphaBeta(depth - 1, alpha, beta, maximizingSide, deadline, timedOut);
            if (timedOut) return 0;
            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, bestScore);
            if (alpha >= beta) break;
        }
        return bestScore;
    }

    int bestScore = std::numeric_limits<int>::max();
    for (const Action& action : actions) {
        GameLogic next = *this;
        next.applyAction(action);
        int score = next.alphaBeta(depth - 1, alpha, beta, maximizingSide, deadline, timedOut);
        if (timedOut) return 0;
        bestScore = std::min(bestScore, score);
        beta = std::min(beta, bestScore);
        if (alpha >= beta) break;
    }
    return bestScore;
}

void GameLogic::aiMoveClassic() {
    std::vector<Action> actions = generateActions(true);

    if (!actions.empty()) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<> dis(0, (int)actions.size() - 1);
        Action a = actions[dis(g)];
        applyAction(a);
    }
}

void GameLogic::aiMoveOpenStrong() {
    std::vector<Action> actions = generateActions(false);
    if (actions.empty()) return;

    std::sort(actions.begin(), actions.end(), [&](const Action& a, const Action& b) {
        return scoreActionForOrdering(a) > scoreActionForOrdering(b);
    });

    Action bestAction = actions.front();
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(1400);

    for (int depth = 1; depth <= 5; depth++) {
        if (std::chrono::steady_clock::now() >= deadline) break;

        bool timedOut = false;
        int bestScore = std::numeric_limits<int>::min();
        Action depthBest = bestAction;

        for (const Action& action : actions) {
            GameLogic next = *this;
            next.applyAction(action);
            int score = next.alphaBeta(
                depth - 1,
                std::numeric_limits<int>::min() / 2,
                std::numeric_limits<int>::max() / 2,
                Side::SIDE_BLUE,
                deadline,
                timedOut);

            if (timedOut) break;

            if (score > bestScore ||
                (score == bestScore && scoreActionForOrdering(action) > scoreActionForOrdering(depthBest))) {
                bestScore = score;
                depthBest = action;
            }
        }

        if (timedOut) break;
        bestAction = depthBest;

        std::stable_sort(actions.begin(), actions.end(), [&](const Action& a, const Action& b) {
            if (a.from == bestAction.from && a.to == bestAction.to && a.isFlip == bestAction.isFlip) return true;
            if (b.from == bestAction.from && b.to == bestAction.to && b.isFlip == bestAction.isFlip) return false;
            return scoreActionForOrdering(a) > scoreActionForOrdering(b);
        });
    }

    applyAction(bestAction);
}

void GameLogic::aiMove() {
    if (difficultyMode == DifficultyMode::OPEN_STRONG) aiMoveOpenStrong();
    else aiMoveClassic();
}
